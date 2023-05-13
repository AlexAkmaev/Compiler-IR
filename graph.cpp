#include "include/graph.h"
#include "include/basic_block.h"
#include "passes/include/pass.h"
#include <algorithm>

namespace compiler {

size_t Graph::instrs_count_ = 0;

BasicBlock *Graph::FindBlock(size_t id) {
    auto dfs_blocks = passes::Traversal{this}.getDFS(true);
    auto it = std::find_if(dfs_blocks.begin(), dfs_blocks.end(),
                        [id](BasicBlock *bb) { return bb->GetId() == id; });
    return *it;
}

void Graph::SetGraphForBasicBlocks(std::initializer_list<BasicBlock *> bbs) {
    for (auto bb: bbs) {
        bb->SetGraph(this);
    }
    InvalidateRpo();
    InvalidateDomTree();
    InvalidateLoopAnalysis();
}

BasicBlock *Graph::RemoveBlock(size_t id) {
    auto *rm_bb = FindBlock(id);
    assert(rm_bb != nullptr);
    for (auto *bb : rm_bb->GetPreds()) {
        bb->RemoveFromSuccs(rm_bb->GetId());
    }
    return rm_bb;
}

void Graph::RestoreBlock(BasicBlock *rm_bb) {
    for (auto *bb : rm_bb->GetPreds()) {
        bb->AddToSuccs({rm_bb});
    }
}

void Graph::AddLabel(const std::string &label) {
    auto it = label_table_.find(label);
    if (it == label_table_.end()) {
        label_table_.emplace(label, label_table_.size());
    }
}

std::optional<size_t> Graph::GetLabelId(const std::string &label) {
    auto it = label_table_.find(label);
    if (it != label_table_.end()) {
        return label_table_.at(label);
    }
    return {};
}

void Graph::AddTarget(const std::string &label, InstructionBase *instr) {
    jump_table_.try_emplace(label, instr);
}

std::optional<InstructionBase *> Graph::GetTargetInstr(const std::string &label) {
    auto it = jump_table_.find(label);
    if (it != jump_table_.end()) {
        return jump_table_.at(label);
    }
    return {};
}

}  // namespace compiler
