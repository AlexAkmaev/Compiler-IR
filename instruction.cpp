#include "instruction.h"
#include "basic_block.h"

namespace compiler {

Graph *InstructionBase::GetGraph() {
    assert(bb_ != nullptr && "BasicBlock inside instr must not be nullptr.\n");
    return bb_->GetGraph();
}

}  // namespace compiler
