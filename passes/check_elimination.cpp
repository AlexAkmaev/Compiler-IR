#include "check_elimination.h"

namespace compiler::passes {

bool CheckElimination::Run() {
    if (graph_ == nullptr) {
        std::cerr << "Error! Graph is nullptr." << std::endl;
        return false;
    }
    passes::Traversal tr{graph_};
    BlocksVector rpo = tr.getRPO(true);
    passes::DomTree domTree{graph_, true};
    for (auto *bb: rpo) {
        for (auto *instr: bb->GetAllInstrs()) {
            if (!instr->IsCheck()) {
                continue;
            }
            RemoveDominatedChecks(instr);
        }
    }
    return true;
}

void CheckElimination::RemoveDominatedChecks(InstructionBase *check) {
    if (check->IsBoundsCheck()) {
        RemoveDominatedBoundsCheck(check);
        return;
    }
    assert(check->GetInputs().size() == 1);
    auto *input = check->GetInputs().front()->def();
    for (auto *i_user: input->GetUsers()) {
        if (i_user->IsSameOpcode(check) && i_user->IsDominatedBy(check)) {
            input->RemoveUser(i_user);
            i_user->ReplaceInputForUsers(check);
            i_user->MakeNop();
        }
    }
}

void CheckElimination::RemoveDominatedBoundsCheck(InstructionBase *check) {
    assert(check->GetInputs().size() == 2);
    auto *len_array = check->GetInputs().at(0)->def();
    auto *idx = check->GetInputs().at(1)->def();
    for (auto *i_user: len_array->GetUsers()) {
        bool is_similar_bounds_check =
                i_user->GetInputs().at(0)->def() == len_array && i_user->GetInputs().at(1)->def() == idx;
        if (i_user->IsSameOpcode(check) && is_similar_bounds_check && i_user->IsDominatedBy(check)) {
            len_array->RemoveUser(i_user);
            i_user->ReplaceInputForUsers(check);
            i_user->MakeNop();
        }
    }
}

}
