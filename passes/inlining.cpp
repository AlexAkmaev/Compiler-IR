#include "inlining.h"
#include "instruction.h"

namespace compiler::passes {

bool Inlining::Run() {
    if (graph_ == nullptr) {
        std::cout << "Warning! Graph is null, so not inlined.\n";
        return false;
    }
    if (bbs_in_rpo_.empty()) {
        bbs_in_rpo_ = passes::Traversal{graph_}.getRPO(true);
    }
    InsnsVec call_insns;
    if (!IsGraphSuitableForInl(call_insns)) {
        std::cout << "Warning! Graph is not suitable for inline.\n";
        return false;
    }
    for (auto *insn: call_insns) {
        DoInlineMethod(insn);
    }
    return true;
}

bool Inlining::IsGraphSuitableForInl(InsnsVec &call_insns) {
    size_t insns_count = 0;
    for (auto *bb: bbs_in_rpo_) {
        for (auto *insn: bb->GetAllInstrs()) {
            if (insn->IsCall()) {
                call_insns.push_back(insn);
            }
        }
        insns_count += bb->GetAllInstrs().size();
    }
    if (insns_count > insns_limit_) {
        return false;
    }
    return !call_insns.empty();
}

void Inlining::DoInlineMethod(InstructionBase *caller) {
    Graph *inlined_graph = caller->GetInputs().front()->callee();  // first input is foo label and graph
    assert(inlined_graph != nullptr && "Error! Callee graph is null.\n");

    auto *cur_bb = caller->GetBasicBlock();
    auto second_bb = cur_bb->SplitOn(caller);

    // Move inlined parameters' users to caller inputs' users
    if (caller->HasInputs()) {
        InsnsVec inlined_params;
        InsnsVec start_block_instrs = inlined_graph->GetRoot()->GetAllInstrs();
        std::copy_if(start_block_instrs.begin(), start_block_instrs.end(),
                     std::back_inserter(inlined_params),
                     [](InstructionBase *instr) { return instr->GetOpcode() == Opcode::PARAMETER; });
        auto call_args = caller->GetInputs();
        call_args.erase(call_args.begin());  // call label is not arg of the function
        assert(call_args.size() == inlined_params.size());
        for (size_t param_idx = 0; param_idx < call_args.size(); ++param_idx) {
            auto *caller_input = call_args.at(param_idx)->def();
            auto *inl_param = inlined_params.at(param_idx);
            inl_param->ReplaceInputForUsers(caller_input);
            caller_input->AddUsers(inl_param->GetUsers());
            caller_input->RemoveUser(caller);
        }
    }

    // Convert return or throw instructions to phi function and delete them
    auto end_preds = inlined_graph->GetEnd()->GetPreds();
    size_t inl_ret_count = std::count_if(end_preds.begin(), end_preds.end(), [](BasicBlock *bb) {
        return bb->GetLastInstr()->GetOpcode() == Opcode::RET;
    });
    for (auto *end_pred: end_preds) {
        ReplaceExitDataFlowEdges(caller, end_pred->GetLastInstr(), second_bb, inl_ret_count);
        end_pred->RemoveLastInstr();
    }

    MoveConstants(inlined_graph->GetRoot());

    ChangeInlBlocksRelation(inlined_graph);

    // Connect inlined graph as successor of cur_bb
    assert(inlined_graph->GetRoot()->GetSuccs().size() == 1);  // start block has only one successor
    auto *bb_after_root = inlined_graph->GetRoot()->GetSuccs().front();
    bb_after_root->RemoveFromPreds(inlined_graph->GetRoot()->GetId());
    BasicBlock::AddEdge(cur_bb, bb_after_root);

    // Move predecessors of inlined end block to second_bb
    for (auto *pred_bb: inlined_graph->GetEnd()->GetPreds()) {
        assert(pred_bb->GetSuccs().size() == 1);  // predecessor of end block has only one successor
        pred_bb->RemoveFromSuccs(inlined_graph->GetEnd()->GetId());
        BasicBlock::AddEdge(pred_bb, second_bb);
    }

    cur_bb->RemoveLastInstr();  // remove call instr
}

void Inlining::ReplaceExitDataFlowEdges(InstructionBase *caller, InstructionBase *exit_instr, BasicBlock *second_bb,
                                        size_t inl_ret_count) {
    if (exit_instr->GetOpcode() == Opcode::RET && inl_ret_count > 1) {
        DynamicInputInstr *ret_result;
        if (second_bb->GetFirstInstr()->GetOpcode() == Opcode::PHI) {
            ret_result = static_cast<DynamicInputInstr *>(second_bb->GetFirstInstr());
        } else {
            ret_result = DynamicInputInstr::Create(alloc_, Opcode::PHI, InstrType::U64, {/* acc */});
            second_bb->InsertInstrBefore(second_bb->GetFirstInstr(), ret_result);
            second_bb->SetFirstPhi(ret_result);
        }
        assert(exit_instr->GetInputs().size() == 1);  // ret can be only from one register
        exit_instr->ReplaceUserForInputs(ret_result);
        ret_result->SetInputs(exit_instr->GetInputs());
    } else if (exit_instr->GetOpcode() == Opcode::RET && inl_ret_count == 1) {
        assert(exit_instr->GetInputs().size() == 1);  // ret can be only from one register
        for (auto *user : caller->GetUsers()) {
            exit_instr->ReplaceUserForInputs(user);
        }
        caller->ReplaceInputForUsers(exit_instr->GetInputs().front()->def());
    } else if (exit_instr->GetOpcode() == Opcode::RET_VOID || exit_instr->GetOpcode() == Opcode::THROW) {
        assert(exit_instr->GetInputs().empty());
    } else {
        std::cerr << "Error! Incorrect exit opcode in predecessor of the end block." << std::endl;
    }
}

void Inlining::MoveConstants(BasicBlock *start_block) {
    auto *curr_start_bb = graph_->GetRoot();
    for (auto *instr: start_block->GetAllInstrs()) {
        if (instr->GetOpcode() != Opcode::CONSTANT) {
            continue;
        }
        size_t inl_value = instr->GetDst()->num();  // inlined constant value
        auto curr_instrs = curr_start_bb->GetAllInstrs();
        auto it = std::find_if(curr_instrs.begin(), curr_instrs.end(),
                               [inl_value](InstructionBase *cur_instr) {
                                   return cur_instr->GetOpcode() == Opcode::CONSTANT &&
                                          inl_value == cur_instr->GetDst()->num();
                               });\
        if (it == curr_start_bb->GetAllInstrs().end()) {
            curr_start_bb->InsertInstrAfter(curr_start_bb->GetLastInstr(), *it);
        } else {
            instr->ReplaceInputForUsers(*it);
        }
    }
}

void Inlining::ChangeInlBlocksRelation(Graph *inlined_graph) {
    passes::Traversal tr{inlined_graph};
    BlocksVector rpo = tr.getRPO(true);
    rpo.erase(rpo.begin());  // erase start block
    rpo.pop_back();  // erase end block
    for (auto *inl_bb: rpo) {
        inl_bb->SetGraph(graph_);
        inl_bb->RemoveId();
    }
}

}  // namespace compiler::passes
