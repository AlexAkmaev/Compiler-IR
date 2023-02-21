#include "common.h"
#include "instruction.h"

namespace compiler {

InstrArg::InstrArg(Type type, vreg_t num, Instruction *target) : type_(type), num_(num), target_(target) {
    if (target_ != nullptr) {
        target_->SetIsTarget(true);
    }
}

}  // namespace compiler