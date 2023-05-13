#include "pass.h"

namespace compiler::passes {

class Inlining final : public Pass {
public:
    explicit Inlining(Graph *graph, Allocator *alloc) : Pass(graph), alloc_(alloc) {}

    bool Run() override;

    ~Inlining() override = default;

private:
    bool IsGraphSuitableForInl(InsnsVec &call_insns);

    void DoInlineMethod(InstructionBase *insn);

    void ReplaceExitDataFlowEdges(InstructionBase *caller, InstructionBase *exit_instr, BasicBlock *second_bb,
                                  size_t inl_ret_count);

    void MoveConstants(BasicBlock *start_block);

    void ChangeInlBlocksRelation(Graph *inlined_graph);

    BlocksVector bbs_in_rpo_;

    size_t insns_limit_{50};  // heuristic for amount of instructions inside the inlining graph

    Allocator *alloc_{nullptr};
};

}
