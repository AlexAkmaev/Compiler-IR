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

    static BasicBlock MakeBasicBlock(const std::vector<Instruction *> &instrs);

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

    void SetFirstPhi(Instruction *first_phi) {
        first_phi_ = first_phi;
    }

    Instruction *GetFirstPhi() {
        return first_phi_;
    }

    void AddToPreds(std::initializer_list<BasicBlock *> bbs) {
        preds_.insert(preds_.end(), bbs.begin(), bbs.end());
    }

    void AddToSuccs(std::initializer_list<BasicBlock *> bbs) {
        succs_.insert(succs_.end(), bbs.begin(), bbs.end());
    }

    [[nodiscard]] std::vector<BasicBlock *> GetPreds() const {
        return preds_;
    }

    [[nodiscard]] std::vector<BasicBlock *> GetSuccs() const {
        return succs_;
    }

private:
    Instruction *first_instr_{nullptr};
    Instruction *last_instr_{nullptr};
    Instruction *first_phi_{nullptr};

    std::vector<BasicBlock *> preds_;
    std::vector<BasicBlock *> succs_;

    Graph *graph_;
};

}  // namespace compiler

#endif //OPTIMIZER_BASICBLOCK_H
