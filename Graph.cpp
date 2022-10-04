#include "Graph.h"

namespace compiler {

Graph::Graph(BasicBlock *root, BasicBlock *end, uint8_t params_num) : root_(root), end_(end), params_num_(params_num) {
    for (size_t i = 0; i < params_num; ++i) {
        vreg_table_.emplace(InstrArg{InstrArg::Type::a, static_cast<vreg_t>(i)}, 0);
    }
}

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

void Graph::AddVReg(InstrArg::Type type, vreg_t num, size_t value) {
    vreg_table_.emplace(InstrArg{type, num}, value);
}

std::optional<size_t> Graph::GetVRegValue(InstrArg::Type type, vreg_t num) {
    InstrArg arg{type, num};
    auto it = vreg_table_.find(arg);
    if (it != vreg_table_.end()) {
        return vreg_table_.at(arg);
    }
    return {};
}

}
