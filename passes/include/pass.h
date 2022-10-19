#ifndef OPTIMIZER_PASS_H
#define OPTIMIZER_PASS_H

#include <unordered_set>

#include "Graph.h"

namespace compiler::passes {

class Pass {
public:
    explicit Pass(Graph *graph) : graph_(graph) {}
    virtual bool Run() = 0;
    virtual ~Pass() = default;

protected:
    Graph *graph_;
};

class Traversal final : public Pass {
public:
    explicit Traversal(Graph *graph) : Pass(graph) {}
    bool Run() override;
    ~Traversal() override = default;

    std::vector<BasicBlock *> getDFS();
    std::vector<BasicBlock *> getRPO();

private:
    bool dfsWalk(BasicBlock *bb, std::unordered_set<size_t> &discovered_bbs);

    std::vector<BasicBlock *> dfs_bbs_;
};

}  // namespace compiler::passes

#endif //OPTIMIZER_PASS_H
