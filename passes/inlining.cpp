#include "inlining.h"

namespace compiler::passes {

bool Inlining::Run() {
    if (graph_ == nullptr) {
        std::cout << "Warning! Graph is null, so not inlined.\n";
        return false;
    }
    if (bbs_in_rpo_.empty()) {
        bbs_in_rpo_ = passes::Traversal{graph_}.getRPO(true);
    }
    InsnsVec call_insns;
    if (!IsGraphSuitableForInl(call_insns)) {
        std::cout << "Warning! Graph is not suitable for inline.\n";
        return false;
    }
    for (auto *insn : call_insns) {
        DoInlineMethod(insn);
    }
    return true;
}

bool Inlining::IsGraphSuitableForInl(InsnsVec &call_insns) {
    for (auto *bb : bbs_in_rpo_) {
        for (auto *insn : bb->GetAllInstrs()) {
            if (insn->IsCall()) {
                call_insns.push_back(insn);
            }
        }
    }
    return !call_insns.empty();
}

void Inlining::DoInlineMethod(InstructionBase *insn) {
    Graph *callee = insn->GetArgs().front()->callee();
    assert(callee != nullptr && "Error! Callee graph is null.\n");


}

}  // namespace compiler::passes
