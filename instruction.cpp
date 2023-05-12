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

}  // namespace compiler
