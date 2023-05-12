#include <algorithm>

#include "include/allocator.h"
#include "include/basic_block.h"
#include "include/graph.h"
#include "loop.h"

namespace compiler {

BasicBlock::BasicBlock(Graph *graph) {
    graph_ = graph;
    id_ = graph_->GetBlocksNum();
    graph_->IncreaseBlocksNum();
}

BasicBlock::BasicBlock(InstructionBase *first_instr, InstructionBase *last_instr, Graph *graph) : first_instr_(first_instr),
                                                                                                  last_instr_(last_instr) {
    graph_ = graph;
    id_ = graph_->GetBlocksNum();
    graph_->IncreaseBlocksNum();
}

std::set<size_t> BasicBlock::CollectIds(const BlocksVector &bbs) {
    std::set<size_t> ids;
    for (auto bb: bbs) {
        ids.insert(bb->GetId());
    }
    return ids;
}

size_t BasicBlock::GetId() {
    if (id_.has_value()) {
        return id_.value();
    }
    id_ = graph_->GetBlocksNum();
    graph_->IncreaseBlocksNum();
    return id_.value();
}

BasicBlock BasicBlock::MakeBasicBlock(const std::vector<InstructionBase *> &instrs) {
    BasicBlock bb;
    if (instrs.empty()) {
        return bb;
    }
    InstructionBase *prev = instrs.front();
    assert(prev != nullptr && "prev instruction must not be nullptr");
    bb.SetFirstInstr(prev);
    prev->SetBasicBlock(&bb);
    if (instrs.size() == 1) {
        bb.SetLastInstr(prev);
        return bb;
    }
    InstructionBase *curr;
    for (size_t i = 1; i < instrs.size(); ++i) {
        curr = instrs.at(i);
        assert(curr != nullptr && "curr instruction must not be nullptr");
        prev->SetNext(curr);
        curr->SetPrev(prev);
        curr->SetBasicBlock(&bb);
        prev = curr;
    }
    bb.SetLastInstr(instrs.back());
    return bb;
}

void BasicBlock::RemoveFromSuccs(size_t id) {
    auto it = std::find_if(succs_.begin(), succs_.end(),
                           [id](BasicBlock *bb) { return bb->GetId() == id; });
    succs_.erase(it);
}

void BasicBlock::RemoveFromPreds(size_t id) {
    auto it = std::find_if(preds_.begin(), preds_.end(),
                           [id](BasicBlock *bb) { return bb->GetId() == id; });
    preds_.erase(it);
}

bool BasicBlock::IsDominatedBy(BasicBlock *dom) {
    return (std::find_if(dom_blocks_.begin(), dom_blocks_.end(),
                         [dom](BasicBlock *bb) { return dom->GetId() == bb->GetId(); }) != dom_blocks_.end());
}

bool BasicBlock::IsLoopHeader() const {
    assert(loop_ != nullptr && loop_->GetHeader() != nullptr);
    return loop_->GetHeader()->GetId() == id_;
}

InsnsVec BasicBlock::GetAllInstrs() {
    InsnsVec instrs;
    InstructionBase *it_instr = first_instr_;
    while (it_instr != nullptr) {
        instrs.push_back(it_instr);
        it_instr = it_instr->GetNext();
    }
    return instrs;
}

BasicBlock *BasicBlock::SplitOn(InstructionBase *insn) {
    if (insn->GetBasicBlock() != this) {
        std::cerr << "Error! Cannot split basic block on the instruction that is not from this block.\n";
        return {};
    }
    InsnsVec second_bb_instrs;
    InstructionBase *it_instr = insn->GetNext();
    while (it_instr != nullptr) {
        second_bb_instrs.push_back(it_instr);
        it_instr = it_instr->GetNext();
    }
    insn->SetNext(nullptr);
    SetLastInstr(insn);
    BasicBlock *second_bb = graph_->GetAllocator()->New(MakeBasicBlock(second_bb_instrs));
    AddEdge(this, second_bb);
    return second_bb;
}

void BasicBlock::InsertInstrBefore(InstructionBase *bb_instr, InstructionBase *instr) {
    instr->SetBasicBlock(this);
    if (bb_instr->GetPrev() != nullptr) {
        bb_instr->GetPrev()->SetNext(instr);
        instr->SetPrev(bb_instr->GetPrev());
    }
    instr->SetNext(bb_instr);
    bb_instr->SetPrev(instr);
}

void BasicBlock::InsertInstrAfter(InstructionBase *bb_instr, InstructionBase *instr) {
    instr->SetBasicBlock(this);
    if (bb_instr->GetNext() != nullptr) {
        bb_instr->GetNext()->SetPrev(instr);
    }
    instr->SetNext(bb_instr->GetNext());
    instr->SetPrev(bb_instr);
    bb_instr->SetNext(instr);
}

}  // namespace compiler