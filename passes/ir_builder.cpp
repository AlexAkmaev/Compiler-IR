#include "ir_builder.h"

namespace compiler::passes {

bool IrBuilder::Run() {
    if (graph_->GetRoot() != nullptr) {
        std::cerr << "Error! Builder is started on non-empty graph\n";
        return false;
    }
    BuildBasicBlocks();
    return true;
}

void IrBuilder::BuildBasicBlocks() {
    std::vector<Instruction *> buf;
    for (auto *insn : insns_) {
        BasicBlock bb;
        if (insn->IsTarget()) {
            bb = BasicBlock::MakeBasicBlock(buf);
            buf.clear();
            buf.emplace_back(insn);
        } else if (insn->IsControlFlow()) {
            buf.emplace_back(insn);
            bb = BasicBlock::MakeBasicBlock(buf);
            buf.clear();
        } else {
            buf.emplace_back(insn);
            continue;
        }
        bbs_holder_.emplace_back(bb);
    }

    ConnectBasicBlocks();
}

void IrBuilder::ConnectBasicBlocks() {
    graph_->SetRoot(&bbs_holder_.front());
    for (size_t i = 0; i < bbs_holder_.size(); ++i) {
        BasicBlock &bb = bbs_holder_.at(i);
        bb.SetGraph(graph_);
        Instruction *last_instr = bb.GetLastInstr();
        if (last_instr->IsJump() || last_instr->IsConditionalBranch()) {
            auto args = last_instr->GetArgs();
            if (args.size() != 1 || args.front().target() == nullptr) {
                std::cerr << "Error! Wrong instruction format for jump\n";
                return;
            }
            BasicBlock *target_bb = args.front().target()->GetBasicBlock();
            assert(target_bb != nullptr && "Error! BasicBlock must not be nullptr\n");
            target_bb->SetGraph(graph_);
            BasicBlock::AddEdge(&bb, target_bb);
        }
        if (!last_instr->IsReturn()) {
            BasicBlock &next_bb = bbs_holder_.at(i + 1);
            next_bb.SetGraph(graph_);
            BasicBlock::AddEdge(&bb, &next_bb);
        }
    }
    bbs_in_rpo_ = passes::Traversal{graph_}.getRPO(true);
    graph_->SetEnd(bbs_in_rpo_.back());
}

}