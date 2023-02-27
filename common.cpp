#include "common.h"
#include "instruction.h"

namespace compiler {

InstrArg::InstrArg(Type type, vreg_t num, InstructionBase *ref) : type_(type), num_(num), ref_(ref) {
    if (type == Type::id) {
        ref_->SetIsTarget(true);
    }
}

}  // namespace compiler
