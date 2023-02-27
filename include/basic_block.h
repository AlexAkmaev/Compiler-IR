#ifndef OPTIMIZER_BASICBLOCK_H
#define OPTIMIZER_BASICBLOCK_H

#include <set>

#include "instruction.h"
#include "marker.h"

namespace compiler {

class Graph;
class Loop;

class BasicBlock final {
public:
    BasicBlock() = default;

    explicit BasicBlock(InstructionBase *first_instr, InstructionBase *last_instr) : first_instr_(first_instr),
                                                                                     last_instr_(last_instr) {}

    explicit BasicBlock(Graph *graph);

    explicit BasicBlock(InstructionBase *first_instr, InstructionBase *last_instr, Graph *graph);

    static BasicBlock MakeBasicBlock(const std::vector<InstructionBase *> &instrs);

    static std::set<size_t> CollectIds(const BlocksVector &bbs);

    void SetGraph(Graph *graph) {
        graph_ = graph;
    }

    Graph *GetGraph() {
        return graph_;
    }

    void SetFirstInstr(InstructionBase *first_instr) {
        first_instr_ = first_instr;
    }

    InstructionBase *GetFirstInstr() {
        return first_instr_;
    }

    void SetLastInstr(InstructionBase *last_instr) {
        last_instr_ = last_instr;
    }

    InstructionBase *GetLastInstr() {
        return last_instr_;
    }

    void SetFirstPhi(DynamicInputInstr *first_phi) {
        first_phi_ = first_phi;
    }

    DynamicInputInstr *GetFirstPhi() {
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

    InsnsVec GetAllInstrs();

    std::pair<BasicBlock *, BasicBlock *> SplitOn(InstructionBase *insn);

    ~BasicBlock() = default;

private:
    Marker marker_;

    std::optional<size_t> id_;

    InstructionBase *first_instr_{nullptr};
    InstructionBase *last_instr_{nullptr};
    DynamicInputInstr *first_phi_{nullptr};

    BlocksVector preds_;
    BlocksVector succs_;

    BlocksVector dom_blocks_;
    BasicBlock *imm_dom_{nullptr};

    Graph *graph_{nullptr};

    Loop *loop_{nullptr};
};

}  // namespace compiler

#endif //OPTIMIZER_BASICBLOCK_H
