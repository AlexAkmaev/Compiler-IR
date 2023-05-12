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
    Graph *inlined_graph = caller->GetDst()->callee();
    assert(inlined_graph != nullptr && "Error! Callee graph is null.\n");

    auto *cur_bb = caller->GetBasicBlock();
    auto second_bb = cur_bb->SplitOn(caller);

    // Move inlined parameters' users to caller inputs' users
    if (caller->HasInputs()) {
        InsnsVec inlined_params;
        std::copy_if(inlined_graph->GetRoot()->GetAllInstrs().begin(), inlined_graph->GetRoot()->GetAllInstrs().end(),
                     std::back_inserter(inlined_params),
                     [](InstructionBase *instr) { return instr->GetOpcode() == Opcode::PARAMETER; });
        assert(caller->GetInputs().size() == inlined_params.size());
        for (size_t param_idx = 0; param_idx < caller->GetInputs().size(); ++param_idx) {
            auto *caller_input = caller->GetInputs().at(param_idx)->def();
            auto *inl_param = inlined_params.at(param_idx);
            assert(caller_input == inl_param->GetInputs().front()->def());
            inl_param->ReplaceInputForUsers(caller_input);
            caller_input->AddUsers(inl_param->GetUsers());
        }
    }

    // Convert return or throw instructions to phi function and delete them
    for (auto *end_succ: graph_->GetEnd()->GetSuccs()) {
        ReplaceExitDataFlowEdges(end_succ->GetLastInstr(), second_bb);
        end_succ->RemoveLastInstr();
    }

    MoveConstants(inlined_graph->GetRoot());

    // Connect inlined graph as successor of cur_bb
    assert(inlined_graph->GetRoot()->GetSuccs().size() == 1);  // start block has only one successor
    inlined_graph->GetRoot()->GetSuccs().front()->AddToPreds({cur_bb});

    // Move predecessors of inlined end block to second_bb
    for (auto *pred_bb: inlined_graph->GetEnd()->GetPreds()) {
        assert(pred_bb->GetSuccs().size() == 1);  // predecessor of end block has only one successor
        pred_bb->RemoveFromSuccs(inlined_graph->GetEnd()->GetId());
        pred_bb->AddToSuccs({second_bb});
    }
}

void Inlining::ReplaceExitDataFlowEdges(InstructionBase *exit_instr, BasicBlock *second_bb) {
    DynamicInputInstr *ret_result = DynamicInputInstr::Create(alloc_, Opcode::PHI, InstrType::U64, {InstrArg::acc});
    switch (exit_instr->GetOpcode()) {
        case Opcode::RET: {
            assert(exit_instr->GetInputs().size() == 1);  // ret can be only from one register
            exit_instr->ReplaceUserForInputs(ret_result);
            ret_result->SetInputs(exit_instr->GetInputs());
        }
        case Opcode::RET_VOID:
        case Opcode::THROW: {
            assert(exit_instr->GetInputs().empty());
        }
        default:
            std::cerr << "Error! Incorrect exit opcode in predecessor of the end block." << std::endl;
    }
    second_bb->InsertInstrBefore(second_bb->GetFirstInstr(), ret_result);
    second_bb->SetFirstPhi(ret_result);
}

void Inlining::MoveConstants(BasicBlock *start_block) {
    auto *curr_start_bb = graph_->GetRoot();
    for (auto *instr: curr_start_bb->GetAllInstrs()) {
        if (instr->GetOpcode() != Opcode::CONSTANT) {
            continue;
        }
        size_t curr_value = instr->GetDst()->num();  // current constant value
        auto it = std::find_if(start_block->GetAllInstrs().begin(), start_block->GetAllInstrs().end(),
                               [curr_value](InstructionBase *inl_instr) {
                                   return inl_instr->GetOpcode() == Opcode::CONSTANT &&
                                          curr_value == inl_instr->GetDst()->num();
                               });
        if (it == start_block->GetAllInstrs().end()) {
            curr_start_bb->InsertInstrAfter(curr_start_bb->GetLastInstr(), *it);
        } else {
            (*it)->ReplaceInputForUsers(instr);
        }
    }
}

}  // namespace compiler::passes
