#ifndef COMPILER_LOOP_H
#define COMPILER_LOOP_H

#include <algorithm>
#include <cassert>

#include "basic_block.h"

namespace compiler {

class Loop {
public:
    explicit Loop(size_t id, BasicBlock *header) : id_(id), header_(header) {}

    [[nodiscard]] size_t GetId() const {
        return id_;
    }

    [[nodiscard]] BasicBlock *GetHeader() const {
        return header_;
    }

    void SetPreHeader(BasicBlock *preheader) {
        preheader_ = preheader;
    }

    [[nodiscard]] BasicBlock *GetPreHeader() const {
        return preheader_;
    }

    void AddBackEdge(BasicBlock *edge_bb) {
        assert(std::find_if(back_edges_.begin(), back_edges_.end(),
                            [edge_bb](BasicBlock *bb) { return edge_bb->GetId() == bb->GetId(); }) ==
               back_edges_.end());
        back_edges_.push_back(edge_bb);
    }

    [[nodiscard]] BlocksVector GetBackEdges() const {
        return back_edges_;
    }

    void AddLoopBlock(BasicBlock *block) {
        assert(std::find_if(blocks_.begin(), blocks_.end(),
                            [block](BasicBlock *bb) { return block->GetId() == bb->GetId(); }) ==
               blocks_.end());
        block->SetLoop(this);
        blocks_.push_back(block);
    }

    [[nodiscard]] BlocksVector GetLoopBlocks() const {
        return blocks_;
    }

    void SetOutLoop(Loop *out_loop) {
        out_loop_ = out_loop;
    }

    [[nodiscard]] Loop *GetOutLoop() const {
        return out_loop_;
    }

    void AddInLoop(Loop *out_loop) {
        assert(std::find_if(in_loops_.begin(), in_loops_.end(),
                            [out_loop](Loop *loop) { return out_loop->GetId() == loop->GetId(); }) ==
               in_loops_.end());
        in_loops_.push_back(out_loop);
    }

    [[nodiscard]] std::vector<Loop *> GetInLoops() const {
        return in_loops_;
    }

    void MarkAsRoot() {
        is_root_ = true;
    }

    [[nodiscard]] bool IsRoot() const {
        return is_root_;
    }

    void SetIrreducible() {
        is_irreducible_ = true;
    }

    [[nodiscard]] bool IsIrreducible() const {
        return is_irreducible_;
    }

private:
    size_t id_;

    BasicBlock *header_{nullptr};
    BasicBlock *preheader_{nullptr};
    BlocksVector back_edges_;
    BlocksVector blocks_;
    std::vector<Loop *> in_loops_;
    Loop *out_loop_{nullptr};

    bool is_irreducible_{false};
    bool is_root_{false};
};

}   // namespace compiler

#endif //COMPILER_LOOP_H
