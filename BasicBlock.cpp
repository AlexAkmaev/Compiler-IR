#include "BasicBlock.h"

namespace compiler {

BasicBlock BasicBlock::MakeBasicBlock(const std::vector<Instruction *> &instrs) {
    BasicBlock bb;
    if (instrs.empty()) {
        return bb;
    }
    Instruction *prev = instrs.front();
    bb.SetFirstInstr(prev);
    prev->SetBasicBlock(&bb);
    if (instrs.size() == 1) {
        return bb;
    }
    Instruction *curr;
    for (size_t i = 1; i < instrs.size(); ++i) {
        curr = instrs.at(i);
        prev->SetNext(curr);
        curr->SetPrev(prev);
        curr->SetBasicBlock(&bb);
        prev = curr;
    }
    bb.SetLastInstr(instrs.back());
    return bb;
}

}  // namespace compiler