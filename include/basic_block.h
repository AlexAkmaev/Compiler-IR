#ifndef OPTIMIZER_BASICBLOCK_H
#define OPTIMIZER_BASICBLOCK_H

#include "instruction.h"
#include "marker.h"
#include <set>

namespace compiler {

class Graph;
class Loop;
class BasicBlock;

using BlocksVector = std::vector<BasicBlock *>;

class BasicBlock final {
public:
    BasicBlock() = default;

    explicit BasicBlock(Instruction *first_instr, Instruction *last_instr) : first_instr_(first_instr),
                                                                             last_instr_(last_instr) {}

    explicit BasicBlock(Graph *graph);

    explicit BasicBlock(Instruction *first_instr, Instruction *last_instr, Graph *graph);

    static BasicBlock MakeBasicBlock(const std::vector<Instruction *> &instrs);

    static std::set<size_t> CollectIds(const BlocksVector &bbs);

    void SetGraph(Graph *graph) {
        graph_ = graph;
    }

    Graph *GetGraph() {
        return graph_;
    }

    void SetFirstInstr(Instruction *first_instr) {
        first_instr_ = first_instr;
    }

    Instruction *GetFirstInstr() {
        return first_instr_;
    }

    void SetLastInstr(Instruction *last_instr) {
        last_instr_ = last_instr;
    }

    Instruction *GetLastInstr() {
        return last_instr_;
    }

    void SetFirstPhi(PhiInstruction *first_phi) {
        first_phi_ = first_phi;
    }

    PhiInstruction *GetFirstPhi() {
        return first_phi_;
    }

    void SetId(size_t id) {
        id_ = id;
    }

    size_t GetId();

    static void AddEdge(BasicBlock *lhs, BasicBlock *rhs) {
        lhs->AddToSuccs({rhs});
        rhs->AddToPreds({lhs});
    }

    void AddColor(const Marker::Color &c) {
        marker_ |= c;
    }

    void RemoveColor(const Marker::Color &c) {
        marker_ &= c;
    }

    [[nodiscard]] Marker GetMarker() const {
        return marker_;
    }

    void AddToPreds(std::initializer_list<BasicBlock *> bbs) {
        preds_.insert(preds_.end(), bbs.begin(), bbs.end());
    }

    void AddToSuccs(std::initializer_list<BasicBlock *> bbs) {
        succs_.insert(succs_.end(), bbs.begin(), bbs.end());
    }

    void RemoveFromSuccs(size_t id);

    void RemoveFromPreds(size_t id);

    [[nodiscard]] BlocksVector GetPreds() const {
        return preds_;
    }

    [[nodiscard]] BlocksVector GetSuccs() const {
        return succs_;
    }

    void AddToDoms(std::initializer_list<BasicBlock *> bbs) {
        dom_blocks_.insert(dom_blocks_.end(), bbs.begin(), bbs.end());
    }

    void SetImmDom(BasicBlock *bb) {
        imm_dom_ = bb;
    }

    [[nodiscard]] BlocksVector GetDomBlocks() const {
        return dom_blocks_;
    }

    BasicBlock *GetImmDom() {
        return imm_dom_;
    }

    bool IsDominatedBy(BasicBlock *dom);

    void SetLoop(Loop *loop) {
        loop_ = loop;
    }

    Loop *GetLoop() {
        return loop_;
    }

    bool IsLoopHeader() const;

private:
    Marker marker_;

    std::optional<size_t> id_;

    Instruction *first_instr_{nullptr};
    Instruction *last_instr_{nullptr};
    PhiInstruction *first_phi_{nullptr};

    BlocksVector preds_;
    BlocksVector succs_;

    BlocksVector dom_blocks_;
    BasicBlock *imm_dom_;

    Graph *graph_;

    Loop *loop_{nullptr};
};

}  // namespace compiler

#endif //OPTIMIZER_BASICBLOCK_H
