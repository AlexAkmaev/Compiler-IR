#ifndef OPTIMIZER_INSTRUCTION_H
#define OPTIMIZER_INSTRUCTION_H

#include <utility>
#include <vector>
#include <string>
#include "common.h"

namespace compiler {

class BasicBlock;

class Instruction final {
public:
    explicit Instruction(Opcode op) : op_(op) {}

    Instruction(Opcode op, InstrType type, std::vector<InstrArg> args, std::string id) : op_(op),
                                                                                         type_(type),
                                                                                         args_(std::move(args)),
                                                                                         id_(std::move(id)) {}

    Instruction(Opcode op, InstrType type, std::vector<InstrArg> args, std::string id,
                Instruction *prev,
                Instruction *next) : op_(op), type_(type), args_(std::move(args)), id_(std::move(id)), prev_(prev),
                                     next_(next) {}

    void SetArgs(const std::vector<InstrArg> &args) {
        args_ = args;
    }

    std::vector<InstrArg> &GetArgs() {
        return args_;
    }

    void SetId(const std::string &id) {
        id_ = id;
    }

    std::string GetId() {
        return id_;
    }

    void SetType(InstrType type) {
        type_ = type;
    }

    InstrType GetType() {
        return type_;
    }

    void SetPrev(Instruction *prev) {
        prev_ = prev;
    }

    Instruction *GetPrev() {
        return prev_;
    }

    void SetNext(Instruction *next) {
        next_ = next;
    }

    Instruction *GetNext() {
        return next_;
    }

    void SetBasicBlock(BasicBlock *bb) {
        bb_ = bb;
    }

    BasicBlock *GetBasicBlock() {
        return bb_;
    }

private:
    Opcode op_{Opcode::NONE};
    InstrType type_{InstrType::I32};

    std::vector<InstrArg> args_;
    std::string id_;

    Instruction *prev_{nullptr};
    Instruction *next_{nullptr};

    BasicBlock *bb_{nullptr};
};

}  // namespace compiler

#endif //OPTIMIZER_INSTRUCTION_H
