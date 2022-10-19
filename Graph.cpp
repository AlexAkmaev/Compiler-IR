#include "Graph.h"

namespace compiler {

void Graph::SetGraphForBasicBlocks(std::initializer_list<BasicBlock *> bbs) {
    for (auto bb : bbs) {
        bb->SetGraph(this);
    }
}

void Graph::AddLabel(const std::string &label) {
    auto it = label_table_.find(label);
    if (it == label_table_.end()) {
        label_table_.emplace(label, label_table_.size());
    }
}

std::optional<size_t> Graph::GetLabelId(const std::string &label) {
    auto it = label_table_.find(label);
    if (it != label_table_.end()) {
        return label_table_.at(label);
    }
    return {};
}

void Graph::AddTarget(const std::string &label, Instruction *instr) {
    jump_table_.try_emplace(label, instr);
}

std::optional<Instruction *> Graph::GetTargetInstr(const std::string &label) {
    auto it = jump_table_.find(label);
    if (it != jump_table_.end()) {
        return jump_table_.at(label);
    }
    return {};
}

}  // namespace compiler
