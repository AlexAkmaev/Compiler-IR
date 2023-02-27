#include "pass.h"

namespace compiler::passes {

class Inlining final : public Pass {
public:
    explicit Inlining(Graph *graph) : Pass(graph) {}
    bool Run() override;
    ~Inlining() override = default;

private:
    bool IsGraphSuitableForInl(InsnsVec &call_insns);
    void DoInlineMethod(InstructionBase *insn);

    BlocksVector bbs_in_rpo_;

    size_t insns_limit_{50};  // heuristic for amount of instructions inside the inlining graph
};

}
