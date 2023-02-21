#ifndef COMPILER_IR_BUILDER_H
#define COMPILER_IR_BUILDER_H

#include "pass.h"

namespace compiler::passes {

class IrBuilder final : public Pass {
public:
    explicit IrBuilder(Graph *graph, std::vector<Instruction *> insns) : Pass(graph), insns_(std::move(insns)) {}
    bool Run() override;
    ~IrBuilder() override = default;

    std::vector<BasicBlock *> GetBlocks() {
        if (bbs_pointers_.empty()) {
            std::transform(bbs_holder_.begin(), bbs_holder_.end(), bbs_pointers_.begin(), [](BasicBlock &bb) {
                return &bb;
            });
        }
        return bbs_pointers_;
    }

private:
    void BuildBasicBlocks();
    void ConnectBasicBlocks();

    std::vector<Instruction *> insns_;
    std::vector<BasicBlock> bbs_holder_;
    BlocksVector bbs_pointers_;
    BlocksVector bbs_in_rpo_;
};

}  // namespace compiler::passes

#endif //COMPILER_IR_BUILDER_H
