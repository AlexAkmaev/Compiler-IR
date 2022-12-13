#include "include/pass.h"

namespace compiler::passes {

/*================================================================*/
/*========================= Traversal ============================*/
/*================================================================*/
bool Traversal::Run() {
    dfs_bbs_.reserve(graph_->GetBlocksNum());
    IdSet discovered_bbs;
    bool res = DFSWalk(graph_->GetRoot(), discovered_bbs);
    if (res) {
        graph_->MakeRpoValid();
    } else {
        std::cerr << "Error! DFS Walk went wrong\n";
    }
    return res;
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
    if (need_to_rerun || !graph_->IsRpoValid()) {
        Run();
    }
    return dfs_bbs_;
}

BlocksVector Traversal::getRPO(bool need_to_rerun) {
    if (need_to_rerun || !graph_->IsRpoValid()) {
        Run();
    }
    BlocksVector rpo_bbs = dfs_bbs_;
    std::reverse(rpo_bbs.begin(), rpo_bbs.end());
    return rpo_bbs;
}

/*==============================================================*/
/*========================= DomTree ============================*/
/*==============================================================*/
bool DomTree::Run() {
    bool res = is_slow_ ? SlowDomTree() : FastDomTree();
    if (res) {
        graph_->MakeDomTreeValid();
    }
    return res;
}

bool DomTree::SlowDomTree() {
    auto dfs_blocks = Traversal{graph_}.getDFS(true);
    for (auto bb: dfs_blocks) {
        bb->AddToDoms({graph_->GetRoot()});
        for (auto id: CalcDifference(graph_, bb->GetId(), BasicBlock::CollectIds(dfs_blocks))) {
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

/*===================================================================*/
/*========================= LoopAnalyzer ============================*/
/*===================================================================*/
bool LoopAnalyzer::Run() {
    if (!graph_->IsDomTreeValid()) {
        passes::DomTree domTree{graph_, true};
        if (!domTree.Run()) {
            std::cerr << "Error! Dominator tree building is corrupted\n";
            return false;
        }
    }
    assert(graph_->IsDomTreeValid());
    CleanMarkers();
    if (!CollectBackEdges(graph_->GetRoot())) {
        std::cerr << "Error! CollectBackEdges went wrong\n";
        return false;
    }
    CleanMarkers();
    if (!PopulateLoops()) {
        std::cerr << "Error! PopulateLoops went wrong\n";
        return false;
    }
    if (!BuildLoopTree()) {
        std::cerr << "Error! BuildLoopTree went wrong\n";
        return false;
    }
    graph_->MakeLoopAnalysisValid();
    return true;
}

bool LoopAnalyzer::CollectBackEdges(BasicBlock *bb) {
    bb->AddColor(Marker::Color::GREY);
    bb->AddColor(Marker::Color::BLACK);
    for (auto *succ: bb->GetSuccs()) {
        if (succ->GetMarker().HasGrey()) {
            CreateNewBackEdge(succ, bb);
        } else if (!succ->GetMarker().HasBlack()) {
            CollectBackEdges(succ);
        }
    }
    bb->RemoveColor(Marker::Color::GREY);
    return true;
}

void LoopAnalyzer::CreateNewBackEdge(BasicBlock *header, BasicBlock *back_edge) {
    auto *loop = header->GetLoop();
    if (loop == nullptr) {
        loop = AllocateLoop(header);
    }

    loop->AddBackEdge(back_edge);
    if (!back_edge->IsDominatedBy(header)) {
        loop->SetIrreducible();
    }
}

Loop *LoopAnalyzer::AllocateLoop(BasicBlock *header) {
    size_t loop_id = holder_.size();
    assert(std::find_if(holder_.begin(), holder_.end(),
                        [loop_id](const Loop &loop) { return loop_id == loop.GetId(); }) ==
           holder_.end());
    holder_.emplace_back(loop_id, header);
    holder_.back().AddLoopBlock(header);
    return &holder_.back();
}

void LoopAnalyzer::CleanMarkers() {
    auto dfs_blocks = Traversal{graph_}.getDFS(true);
    for (auto *bb: dfs_blocks) {
        bb->RemoveColor(Marker::Color::GREY);
        bb->RemoveColor(Marker::Color::BLACK);
    }
}

bool LoopAnalyzer::PopulateLoops() {
    auto dfs_blocks = Traversal{graph_}.getDFS(true);
    for (auto bb: dfs_blocks) {
        if (bb->GetLoop() == nullptr || !bb->IsLoopHeader()) {
            continue;
        }
        auto loop = bb->GetLoop();
        if (loop->IsIrreducible()) {
            for (auto back_edge : loop->GetBackEdges()) {
                if (back_edge->GetLoop() != loop) {
                    loop->AddLoopBlock(back_edge);
                }
            }
        } else {
            bb->AddColor(Marker::Color::BLACK);
            for (auto back_edge : loop->GetBackEdges()) {
                if (!LoopSearch(back_edge, loop)) {
                    std::cerr << "Error! LoopSearch went wrong\n";
                    return false;
                }
            }
            CleanMarkers();
        }
    }
    return true;
}

bool LoopAnalyzer::LoopSearch(BasicBlock *bb, Loop *loop) {
    if (!bb->GetMarker().HasBlack()) {
        bb->AddColor(Marker::Color::BLACK);

        if (bb->GetLoop() == nullptr) {
            loop->AddLoopBlock(bb);
        } else if (bb->GetLoop()->GetHeader() != loop->GetHeader()) {
            if (bb->GetLoop()->GetOutLoop() == nullptr) {
                bb->GetLoop()->SetOutLoop(loop);
                loop->AddInLoop(bb->GetLoop());
            }
        }

        for (auto *pred : bb->GetPreds()) {
            if (!LoopSearch(pred, loop)) {
                std::cerr << "Error! LoopSearch for predecessor went wrong\n";
                return false;
            }
        }
    }
    return true;
}

bool LoopAnalyzer::BuildLoopTree() {
    Loop *root_loop = CreateRootLoop();
    auto dfs_blocks = Traversal{graph_}.getDFS(true);
    for (auto bb: dfs_blocks) {
        if (bb->GetLoop() == nullptr) {
            root_loop->AddLoopBlock(bb);
        } else if (bb->GetLoop()->GetOutLoop() == nullptr) {
            bb->GetLoop()->SetOutLoop(root_loop);
            root_loop->AddInLoop(bb->GetLoop());
        }
    }
    return true;
}

Loop *LoopAnalyzer::CreateRootLoop() {
    Loop *root_loop;
    if (graph_->GetRoot()->GetLoop() != nullptr) {
        root_preheader_ = BasicBlock{};
        graph_->MoveRoot(&root_preheader_);
        root_loop = AllocateLoop(&root_preheader_);
        root_loop->SetPreHeader(&root_preheader_);
    } else {
        root_loop = AllocateLoop(graph_->GetRoot());
    }
    root_loop->MarkAsRoot();
    graph_->GetRoot()->SetLoop(root_loop);
    return root_loop;
}

}  // namespace compiler::passes
