#include "Graph.h"
#include "BasicBlock.h"
#include "pass.h"
#include <algorithm>

namespace compiler {

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
}

void Graph::RemoveBlock(size_t id) {
    auto *rm_bb = FindBlock(id);
    for (auto *bb : rm_bb->GetPreds()) {
        bb->RemoveFromSuccs(rm_bb->GetId());
    }
    for (auto *bb : rm_bb->GetSuccs()) {
        bb->RemoveFromPreds(rm_bb->GetId());
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

void Graph::AddTarget(const std::string &label, Instruction *instr) {
    jump_table_.try_emplace(label, instr);
}

std::optional<Instruction *> Graph::GetTargetInstr(const std::string &label) {
    auto it = jump_table_.find(label);
    if (it != jump_table_.end()) {
        return jump_table_.at(label);
    }
    return {};
}

}  // namespace compiler
