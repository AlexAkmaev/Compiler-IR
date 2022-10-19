#ifndef OPTIMIZER_INSTRUCTION_H
#define OPTIMIZER_INSTRUCTION_H

#include <utility>
#include <vector>
#include <string>
#include "common.h"

namespace compiler {

class BasicBlock;

class Instruction {
public:
    explicit Instruction(Opcode op) : op_(op) {}

    Instruction(Opcode op, InstrType type, std::vector<InstrArg> args, size_t id) : op_(op),
                                                                                    type_(type),
                                                                                    args_(std::move(args)),
                                                                                    id_(id) {}

    Instruction(Opcode op, InstrType type, std::vector<InstrArg> args, size_t id,
                Instruction *prev,
                Instruction *next) : op_(op), type_(type), args_(std::move(args)), id_(id), prev_(prev),
                                     next_(next) {}

    void SetArgs(const std::vector<InstrArg> &args) {
        args_ = args;
    }

    std::vector<InstrArg> &GetArgs() {
        return args_;
    }

    void SetId(size_t id) {
        id_ = id;
    }

    size_t GetId() {
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

protected:
    Opcode op_{Opcode::NONE};
    InstrType type_{InstrType::I32};

    std::vector<InstrArg> args_;
    size_t id_;

    Instruction *prev_{nullptr};
    Instruction *next_{nullptr};

    std::vector<Instruction *> uses_;

    BasicBlock *bb_{nullptr};

private:
    Instruction *def_;
};

class PhiInstruction final : public Instruction {
public:
    template<typename... Defs>
    static PhiInstruction CreatePhi(InstrType type, size_t id, Defs... defs) {
        return PhiInstruction(type, id, std::vector<Instruction *>{{defs...}});
    }

    template<typename... Defs>
    static PhiInstruction CreatePhi(InstrType type, size_t id, Instruction *prev, Instruction *next, Defs... defs) {
        return PhiInstruction(type, id, prev, next, std::vector<Instruction *>{{defs...}});
    }

private:
    PhiInstruction(InstrType type, size_t id, std::vector<Instruction *> &&defs) : Instruction(Opcode::PHI, type, {},
                                                                                               id),
                                                                                   defs_(std::move(defs)) {}

    PhiInstruction(InstrType type, size_t id, Instruction *prev, Instruction *next, std::vector<Instruction *> &&defs)
            : Instruction(Opcode::PHI, type, {}, id, prev, next), defs_(std::move(defs)) {}

    std::vector<Instruction *> defs_;
};

}  // namespace compiler

#endif //OPTIMIZER_INSTRUCTION_H
