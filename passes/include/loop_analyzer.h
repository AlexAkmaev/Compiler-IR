#ifndef COMPILER_LOOP_ANALYZER_H
#define COMPILER_LOOP_ANALYZER_H

#include "pass.h"
#include "loop.h"

namespace compiler::passes {

class LoopAnalyzer final : public Pass {
public:
    explicit LoopAnalyzer(Graph *graph) : Pass(graph) {
        holder_.reserve(graph->GetBlocksNum());  // Allocate memory
    }

    bool Run() override;

    ~LoopAnalyzer() override = default;

private:
    bool CollectBackEdges(BasicBlock *bb);

    void CreateNewBackEdge(BasicBlock *header, BasicBlock *back_edge);

    bool PopulateLoops();

    bool LoopSearch(BasicBlock *bb, Loop *loop);

    bool BuildLoopTree();

    Loop *AllocateLoop(BasicBlock *header);

    void CleanMarkers();

    Loop *CreateRootLoop();

private:
    std::vector <Loop> holder_;
    BasicBlock root_preheader_;
};

}  // namespace compiler::passes

#endif //COMPILER_LOOP_ANALYZER_H
