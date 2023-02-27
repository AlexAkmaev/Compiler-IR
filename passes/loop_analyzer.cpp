#include "loop_analyzer.h"

namespace compiler::passes {

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
