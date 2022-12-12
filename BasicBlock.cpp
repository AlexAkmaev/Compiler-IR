#include "include/BasicBlock.h"
#include "include/Graph.h"
#include "Loop.h"
#include <algorithm>

namespace compiler {

BasicBlock::BasicBlock(Graph *graph) {
    graph_ = graph;
    id_ = graph_->GetBlocksNum();
    graph_->IncreaseBlocksNum();
}

BasicBlock::BasicBlock(Instruction *first_instr, Instruction *last_instr, Graph *graph) : first_instr_(first_instr),
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

BasicBlock BasicBlock::MakeBasicBlock(const std::vector<Instruction *> &instrs) {
    BasicBlock bb;
    if (instrs.empty()) {
        return bb;
    }
    Instruction *prev = instrs.front();
    bb.SetFirstInstr(prev);
    prev->SetBasicBlock(&bb);
    if (instrs.size() == 1) {
        bb.SetLastInstr(prev);
        return bb;
    }
    Instruction *curr;
    for (size_t i = 1; i < instrs.size(); ++i) {
        curr = instrs.at(i);
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
    return loop_->GetHeader()->GetId() == id_;
}

}  // namespace compiler