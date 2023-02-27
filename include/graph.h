#ifndef OPTIMIZER_GRAPH_H
#define OPTIMIZER_GRAPH_H

#include <unordered_map>
#include <map>
#include <optional>
#include <cassert>

#include "basic_block.h"

namespace compiler {

class BasicBlock;
class Traversal;

class Graph final {
public:
    Graph(Allocator *alloc) : allocator_(alloc) {}
    explicit Graph(Allocator *alloc, BasicBlock *root, BasicBlock *end, uint8_t params_num) :
                    allocator_(alloc), root_(root), end_(end), params_num_(params_num), blocks_num_{2} {
        root->SetId(0);
        end->SetId(1);
    }

    Allocator *GetAllocator() {
        return allocator_;
    }

    void SetRoot(BasicBlock *root) {
        root->SetId(0);
        root_ = root;
        ++blocks_num_;
    }

    BasicBlock *GetRoot() {
        return root_;
    }

    void MoveRoot(BasicBlock *new_root) {
        BasicBlock::AddEdge(new_root, root_);
        SetGraphForBasicBlocks({new_root});
        root_ = new_root;
    }

    void SetEnd(BasicBlock *end) {
        end->SetId(1);
        end_ = end;
        ++blocks_num_;
    }

    BasicBlock *GetEnd() {
        return end_;
    }

    BasicBlock *FindBlock(size_t id);

    void SetGraphForBasicBlocks(std::initializer_list<BasicBlock *> bbs);

    BasicBlock *RemoveBlock(size_t id);

    void RestoreBlock(BasicBlock *bb);

    bool IsRpoValid() const {
        return rpo_valid_;
    }

    void MakeRpoValid() {
        rpo_valid_ = true;
    }

    void InvalidateRpo() {
        rpo_valid_ = false;
    }

    void SetParamsNum(uint8_t params_num) {
        params_num_ = params_num;
    }

    uint8_t GetParamsNum() const {
        return params_num_;
    }

    void IncreaseBlocksNum() {
        ++blocks_num_;
    }

    void IncreaseBlocksNumOn(uint16_t count) {
        blocks_num_ += count;
    }

    size_t GetBlocksNum() const {
        return blocks_num_;
    }

    bool IsDomTreeValid() const {
        return dom_tree_valid_;
    }

    void MakeDomTreeValid() {
        dom_tree_valid_ = true;
    }

    void InvalidateDomTree() {
        dom_tree_valid_ = false;
    }

    bool IsLoopAnalysisValid() const {
        return loop_analysis_valid_;
    }

    void MakeLoopAnalysisValid() {
        loop_analysis_valid_ = true;
    }

    void InvalidateLoopAnalysis() {
        loop_analysis_valid_ = false;
    }

    Loop *GetRootLoop() {
        assert(loop_analysis_valid_);
        return root_->GetLoop();
    }

    void AddLabel(const std::string &label);

    std::optional<size_t> GetLabelId(const std::string &label);

    void AddTarget(const std::string &label, InstructionBase *instr);

    std::optional<InstructionBase *> GetTargetInstr(const std::string &label);

private:
    Allocator *allocator_;

    BasicBlock *root_;
    BasicBlock *end_;
    uint8_t params_num_;
    size_t blocks_num_{0};
    std::unordered_map<std::string, size_t> label_table_;
    std::unordered_map<std::string, InstructionBase *> jump_table_;

    bool rpo_valid_{false};
    bool dom_tree_valid_{false};
    bool loop_analysis_valid_{false};
};

}  // namespace compiler

#endif //OPTIMIZER_GRAPH_H
