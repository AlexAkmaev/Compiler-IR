#include "pass.h"

namespace compiler::passes {

class CheckElimination final : public Pass {
public:
    explicit CheckElimination(Graph *graph) : Pass(graph) {}

    bool Run() override;

    ~CheckElimination() override = default;

private:
    void RemoveDominatedChecks(InstructionBase *check);
    void RemoveDominatedBoundsCheck(InstructionBase *check);
};

}
