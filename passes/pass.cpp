#include "include/pass.h"
#include <algorithm>

namespace compiler::passes {

bool Traversal::Run() {
    dfs_bbs_.reserve(graph_->GetBlocksNum());
    std::unordered_set<size_t> discovered_bbs;
    return dfsWalk(graph_->GetRoot(), discovered_bbs);
}

bool Traversal::dfsWalk(BasicBlock *bb, std::unordered_set<size_t> &discovered_bbs) {
    if (discovered_bbs.find(bb->GetId()) != discovered_bbs.end()) {
        return true;
    }
    discovered_bbs.insert(bb->GetId());
    for (auto *succ : bb->GetSuccs()) {
        dfsWalk(succ, discovered_bbs);
    }
    dfs_bbs_.emplace_back(bb);
    return true;
}

std::vector<BasicBlock *> Traversal::getDFS() {
    return dfs_bbs_;
}

std::vector<BasicBlock *> Traversal::getRPO() {
    std::vector<BasicBlock *> rpo_bbs = dfs_bbs_;
    std::reverse(rpo_bbs.begin(), rpo_bbs.end());
    return rpo_bbs;
}

}  // namespace compiler::passes
