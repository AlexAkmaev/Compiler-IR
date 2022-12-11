#include "include/pass.h"
#include <algorithm>

namespace compiler::passes {

bool Traversal::Run() {
    dfs_bbs_.reserve(graph_->GetBlocksNum());
    IdSet discovered_bbs;
    return DFSWalk(graph_->GetRoot(), discovered_bbs);
}

bool Traversal::DFSWalk(BasicBlock *bb, IdSet &discovered_bbs) {
    if (discovered_bbs.find(bb->GetId()) != discovered_bbs.end()) {
        return true;
    }
    discovered_bbs.insert(bb->GetId());
    for (auto *succ: bb->GetSuccs()) {
        DFSWalk(succ, discovered_bbs);
    }
    dfs_bbs_.emplace_back(bb);
    return true;
}

BlocksVector Traversal::getDFS(bool need_to_rerun) {
    if (need_to_rerun) {
        Run();
    }
    return dfs_bbs_;
}

BlocksVector Traversal::getRPO(bool need_to_rerun) {
    if (need_to_rerun) {
        Run();
    }
    BlocksVector rpo_bbs = dfs_bbs_;
    std::reverse(rpo_bbs.begin(), rpo_bbs.end());
    return rpo_bbs;
}

bool DomTree::Run() {
    return is_slow_ ? SlowDomTree() : FastDomTree();
}

bool DomTree::SlowDomTree() {
    auto dfs_blocks = Traversal{graph_}.getDFS(true);
    for (auto bb: dfs_blocks) {
        bb->AddToDoms({graph_->GetRoot()});
        for (auto id : CalcDifference(graph_, bb->GetId(), BasicBlock::CollectIds(dfs_blocks))) {
            graph_->FindBlock(id)->AddToDoms({bb});
        }
    }
    // immediate dominators calculation
    for (auto bb: dfs_blocks) {
        auto *immDom = CalcImmDominator(bb->GetDomBlocks());
        if (!immDom) {
            return false;
        }
        bb->SetImmDom(immDom);
    }
    return true;
}

std::set<size_t> DomTree::CalcDifference(Graph *graph, size_t rm_id, const std::set<size_t> &ids) {
    auto *rm_bb = graph->RemoveBlock(rm_id);
    auto new_ids = BasicBlock::CollectIds(Traversal{graph}.getDFS(true));
    std::set<size_t> intersect;
    std::set_difference(ids.begin(), ids.end(), new_ids.begin(), new_ids.end(),
                          std::inserter(intersect, intersect.end()));
    intersect.erase(rm_id);
    graph->RestoreBlock(rm_bb);
    return intersect;
}

BasicBlock *DomTree::CalcImmDominator(const BlocksVector &doms) {
    for (size_t i = 0; i < doms.size(); ++i) {
        bool is_imm_dom = true;
        for (size_t j = 0; j < doms.size(); ++j) {
            if (i == j) {
                continue;
            }
            if (!doms[i]->IsDominatedBy(doms[j])) {
                is_imm_dom = false;
            }
        }
        if (is_imm_dom) {
            return doms[i];
        }
    }
    return nullptr;
}

bool DomTree::FastDomTree() {
    return SlowDomTree();  // TODO(): Implement fast version
}

}  // namespace compiler::passes
