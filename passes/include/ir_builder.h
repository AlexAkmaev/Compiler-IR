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
        return bbs_pointers_;
    }

private:
    void BuildBasicBlocks();
    void ConnectBasicBlocks();

    std::vector<Instruction *> insns_;
    BlocksVector bbs_pointers_;
    BlocksVector bbs_in_rpo_;
};

}  // namespace compiler::passes

#endif //COMPILER_IR_BUILDER_H
