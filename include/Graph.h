#ifndef OPTIMIZER_GRAPH_H
#define OPTIMIZER_GRAPH_H

#include <unordered_map>
#include <map>
#include <optional>

#include "BasicBlock.h"

namespace compiler {

class BasicBlock;

class Graph final {
public:
    explicit Graph(BasicBlock *root, BasicBlock *end, uint8_t params_num);

    void SetRoot(BasicBlock *root) {
        root_ = root;
    }

    BasicBlock *GetRoot() {
        return root_;
    }

    void SetEnd(BasicBlock *end) {
        end_ = end;
    }

    BasicBlock *GetEnd() {
        return end_;
    }

    void SetParamsNum(uint8_t params_num) {
        params_num_ = params_num;
    }

    uint8_t GetParamsNum() const {
        return params_num_;
    }

    void AddLabel(const std::string &label);

    std::optional<size_t> GetLabelId(const std::string &label);

    void AddTarget(const std::string &label, Instruction *instr);

    std::optional<Instruction *> GetTargetInstr(const std::string &label);

    void AddVReg(InstrArg::Type type, vreg_t num, size_t value);

    std::optional<size_t> GetVRegValue(InstrArg::Type type, vreg_t num);

private:
    BasicBlock *root_;
    BasicBlock *end_;
    uint8_t params_num_;
    std::unordered_map<std::string, size_t> label_table_;
    std::unordered_map<std::string, Instruction *> jump_table_;
    std::unordered_map<InstrArg, size_t, hash_instr_arg> vreg_table_;
};

}  // namespace compiler

#endif //OPTIMIZER_GRAPH_H
