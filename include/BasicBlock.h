#ifndef OPTIMIZER_BASICBLOCK_H
#define OPTIMIZER_BASICBLOCK_H

#include "Instruction.h"

namespace compiler {

class Graph;

class BasicBlock final {
public:
    BasicBlock() = default;
    explicit BasicBlock(Instruction *first_instr, Instruction *last_instr) : first_instr_(first_instr),
                                                                             last_instr_(last_instr) {}

    BasicBlock(Graph *graph);
    explicit BasicBlock(Instruction *first_instr, Instruction *last_instr, Graph *graph);

    static BasicBlock MakeBasicBlock(const std::vector<Instruction *> &instrs);

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

    size_t GetId() {
        return id_;
    }

    void AddToPreds(std::initializer_list<BasicBlock *> bbs) {
        preds_.insert(preds_.end(), bbs.begin(), bbs.end());
    }

    void AddToSuccs(std::initializer_list<BasicBlock *> bbs) {
        succs_.insert(succs_.end(), bbs.begin(), bbs.end());
    }

    void AddToDoms(std::initializer_list<BasicBlock *> bbs) {
        dom_blocks_.insert(dom_blocks_.end(), bbs.begin(), bbs.end());
    }

    void RemoveFromSuccs(size_t id);

    void RemoveFromPreds(size_t id);

    [[nodiscard]] std::vector<BasicBlock *> GetPreds() const {
        return preds_;
    }

    [[nodiscard]] std::vector<BasicBlock *> GetSuccs() const {
        return succs_;
    }

private:
    size_t id_{static_cast<size_t>(-1)};

    Instruction *first_instr_{nullptr};
    Instruction *last_instr_{nullptr};
    PhiInstruction *first_phi_{nullptr};

    std::vector<BasicBlock *> preds_;
    std::vector<BasicBlock *> succs_;

    std::vector<BasicBlock *> dom_blocks_;
    BasicBlock *imm_dom_;

    Graph *graph_;
};

}  // namespace compiler

#endif //OPTIMIZER_BASICBLOCK_H
