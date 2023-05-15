#include "instruction.h"
#include "basic_block.h"
#include "graph.h"

namespace compiler {

Graph *InstructionBase::GetGraph() {
    assert(bb_ != nullptr && "BasicBlock inside instr must not be nullptr.\n");
    return bb_->GetGraph();
}

InstructionBase::InstructionBase(Opcode op) : op_(op), id_(Graph::GenInstrId()) {}

InstructionBase::InstructionBase(Opcode op, InstrType type, InstrArg *dst) : op_(op), type_(type),
                                                                             id_(Graph::GenInstrId()),
                                                                             dst_(dst) {}

InstructionBase::InstructionBase(Opcode op, InstrType type, InstructionBase *prev, InstructionBase *next,
                                 InstrArg *dst) : op_(op), type_(type), id_(Graph::GenInstrId()), prev_(prev),
                                                  next_(next), dst_(dst) {}

bool InstructionBase::IsNextTo(InstructionBase *other) const noexcept {
    assert(other != nullptr && bb_ == other->GetBasicBlock());
    if (this == other) {
        return true;
    }
    auto prev = GetPrev();
    while (prev != nullptr) {
        if (prev == other) {
            return true;
        }
        prev = prev->GetPrev();
    }
    return false;
}

bool InstructionBase::IsDominatedBy(InstructionBase *other) const noexcept {
    assert(other != nullptr);
    if (this == other) {
        return true;
    }
    auto other_bb = other->GetBasicBlock();
    return bb_ == other_bb ? IsNextTo(other) : bb_->IsDominatedBy(other_bb);
}

}  // namespace compiler
