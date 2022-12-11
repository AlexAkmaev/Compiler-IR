#ifndef OPTIMIZER_PASS_H
#define OPTIMIZER_PASS_H

#include <unordered_set>
#include <set>

#include "Graph.h"

namespace compiler::passes {

using IdSet = std::unordered_set<size_t>;

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

    BlocksVector getDFS(bool need_to_rerun = false);
    BlocksVector getRPO(bool need_to_rerun = false);

private:
    bool DFSWalk(BasicBlock *bb, IdSet &discovered_bbs);

    BlocksVector dfs_bbs_;
};

class DomTree final : public Pass {
public:
    explicit DomTree(Graph *graph, bool is_slow) : Pass(graph), is_slow_(is_slow) {}
    bool Run() override;
    ~DomTree() override = default;
private:
    bool SlowDomTree();
    bool FastDomTree();

    std::set<size_t> CalcDifference(Graph *graph, size_t rm_id, const std::set<size_t> &ids);

    bool is_slow_{false};
};

}  // namespace compiler::passes

#endif //OPTIMIZER_PASS_H
