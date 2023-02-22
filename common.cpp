#include "common.h"
#include "instruction.h"

namespace compiler {

InstrArg::InstrArg(Type type, vreg_t num, Instruction *target, Graph *graph, Instruction *def) : type_(type), num_(num),
                                                                                                 target_(target),
                                                                                                 callee_(graph),
                                                                                                 def_(def) {
    if (target_ != nullptr) {
        target_->SetIsTarget(true);
    }
}

}  // namespace compiler
