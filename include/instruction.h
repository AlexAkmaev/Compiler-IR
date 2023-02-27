#ifndef OPTIMIZER_INSTRUCTION_H
#define OPTIMIZER_INSTRUCTION_H

#include <utility>
#include <vector>
#include <array>
#include <string>
#include <cassert>

#include "common.h"

namespace compiler {

class BasicBlock;

/**
 *  Cannot be instantiated directly, use derived classes.
 */
class InstructionBase {
public:
    Graph *GetGraph();

    Opcode GetOpcode() const {
        return op_;
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

    void AddUsers(std::initializer_list<InstructionBase *> uses) {
        users_.insert(users_.end(), uses.begin(), uses.end());
    }

    std::vector<InstructionBase *> GetUsers() {
        return users_;
    }

    void SetPrev(InstructionBase *prev) {
        prev_ = prev;
    }

    InstructionBase *GetPrev() {
        return prev_;
    }

    void SetNext(InstructionBase *next) {
        next_ = next;
    }

    InstructionBase *GetNext() {
        return next_;
    }

    void SetBasicBlock(BasicBlock *bb) {
        bb_ = bb;
    }

    BasicBlock *GetBasicBlock() {
        return bb_;
    }

    bool IsControlFlow() {
        return op_ >= Opcode::JA;
    }

    bool IsJump() {
        return op_ == Opcode::JMP;
    }

    bool IsConditionalBranch() {
        return op_ >= Opcode::JA && op_ <= Opcode::JNE;
    }

    bool IsReturn() {
        return op_ == Opcode::RET;
    }

    bool IsCall() {
        return op_ == Opcode::CALL;
    }

    bool IsConstant() {
        return op_ == Opcode::CONSTANT;
    }

    bool IsParameter() {
        return op_ == Opcode::PARAMETER;
    }

    bool IsTarget() const {
        return is_target_;
    }

    void SetIsTarget(bool is_tgt) {
        is_target_ = is_tgt;
    }

    virtual std::vector<InstrArg *> GetArgs() = 0;

    virtual ~InstructionBase() = default;

protected:
    explicit InstructionBase(Opcode op) : op_(op) {}

    InstructionBase(Opcode op, InstrType type, size_t id) : op_(op),
                                                            type_(type),
                                                            id_(id) {}

    InstructionBase(Opcode op, InstrType type, size_t id,
                    InstructionBase *prev,
                    InstructionBase *next) : op_(op), type_(type), id_(id), prev_(prev),
                                             next_(next) {}


    Opcode op_{Opcode::NONE};
    InstrType type_{InstrType::I32};

    size_t id_{static_cast<size_t>(-1)};

    InstructionBase *prev_{nullptr};
    InstructionBase *next_{nullptr};

    std::vector<InstructionBase *> users_;

    BasicBlock *bb_{nullptr};

private:
    bool is_target_{false};  // is this instruction is target to some jump
};

class DynamicInputInstr final : public InstructionBase {
public:
    template<typename... Args, std::enable_if_t<(std::is_same_v<Args, InstrArg> && ...), bool> = true>
    static DynamicInputInstr *Create(Allocator *alloc, Opcode op, InstrType type, size_t id, Args &&... args) {
        return DynamicInputInstr::Create(alloc, op, type, id, nullptr, nullptr, std::forward<InstrArg>(args)...);
    }

    template<typename... Args, std::enable_if_t<(std::is_same_v<Args, InstrArg> && ...), bool> = true>
    static DynamicInputInstr *
    Create(Allocator *alloc, Opcode op, InstrType type, size_t id, InstructionBase *prev, InstructionBase *next,
           Args &&... args) {
        return alloc->New<DynamicInputInstr>(
                DynamicInputInstr(op, type, id, prev, next, alloc->NewPool<InstrArg>(std::forward<InstrArg>(args)...)));
    }

    void AddArgDefs(std::initializer_list<InstrArg *> args) {
        args_.insert(args_.end(), args.begin(), args.end());
    }

    std::vector<InstrArg *> GetArgs() override {
        return args_;
    }

    ~DynamicInputInstr() override = default;

private:
    DynamicInputInstr(Opcode op, InstrType type, size_t id, std::vector<InstrArg *> &&args) : InstructionBase(op,
                                                                                                              type,
                                                                                                              id),
                                                                                              args_(std::move(args)) {}

    DynamicInputInstr(Opcode op, InstrType type, size_t id, InstructionBase *prev, InstructionBase *next,
                      std::vector<InstrArg *> &&args)
            : InstructionBase(op, type, id, prev, next), args_(std::move(args)) {}

    std::vector<InstrArg *> args_;
};

class ZeroInputInstr final : public InstructionBase {
public:
    static ZeroInputInstr *Create(Allocator *alloc, Opcode op, InstrType type, size_t id) {
        return ZeroInputInstr::Create(alloc, op, type, id, nullptr, nullptr);
    }

    static ZeroInputInstr *
    Create(Allocator *alloc, Opcode op, InstrType type, size_t id, InstructionBase *prev, InstructionBase *next) {
        return alloc->New<ZeroInputInstr>(ZeroInputInstr(op, type, id, prev, next));
    }

    std::vector<InstrArg *> GetArgs() override {
        return {};
    }

    ~ZeroInputInstr() override = default;

private:
    ZeroInputInstr(Opcode op, InstrType type, size_t id) : InstructionBase(op, type, id) {}

    ZeroInputInstr(Opcode op, InstrType type, size_t id, InstructionBase *prev, InstructionBase *next)
            : InstructionBase(op, type, id, prev, next) {}
};

/**
 *  Cannot be instantiated directly, use derived classes.
 *  N - amount of inputs
 */
template<size_t N>
class FixedInputInstr : public InstructionBase {
public:
    virtual ~FixedInputInstr() override = default;

protected:
    template<typename... Args, std::enable_if_t<(std::is_same_v<Args, InstrArg> && ...), bool> = true>
    static FixedInputInstr *Create(Allocator *alloc, Opcode op, InstrType type, size_t id, Args &&... args) {
        return FixedInputInstr::Create(alloc, op, type, id, nullptr, nullptr, std::forward<InstrArg>(args)...);
    }

    template<typename... Args, std::enable_if_t<(std::is_same_v<Args, InstrArg> && ...), bool> = true>
    static FixedInputInstr *Create(Allocator *alloc, Opcode op, InstrType type, size_t id,
                                   InstructionBase *prev, InstructionBase *next, Args &&... args) {
        return alloc->New<FixedInputInstr<N>>(
                FixedInputInstr<N>(op, type, id, prev, next,
                                   alloc->NewPoolAsArray<InstrArg>(std::forward<InstrArg>(args)...)));
    }

    std::vector<InstrArg *> GetArgs() override {
        assert(0 && "Cannot be used for this class");
        return {};
    }

protected:
    std::array<InstrArg *, N> args_;

private:
    FixedInputInstr(Opcode op, InstrType type, size_t id, std::array<InstrArg *, N> &&args) : InstructionBase(op,
                                                                                                              type,
                                                                                                              id),
                                                                                              args_(std::move(args)) {}

    FixedInputInstr(Opcode op, InstrType type, size_t id, InstructionBase *prev, InstructionBase *next,
                    std::array<InstrArg *, N> &&args)
            : InstructionBase(op, type, id, prev, next), args_(std::move(args)) {}

};

class OneInputInstr final : public FixedInputInstr<1> {
public:
    static OneInputInstr *Create(Allocator *alloc, Opcode op, InstrType type, size_t id, InstrArg &&arg) {
        return static_cast<OneInputInstr *>(
                FixedInputInstr<1>::Create(alloc, op, type, id, std::forward<InstrArg>(arg)));
    }

    static OneInputInstr *Create(Allocator *alloc, Opcode op, InstrType type, size_t id,
                                 InstructionBase *prev, InstructionBase *next,
                                 InstrArg &&arg) {
        return static_cast<OneInputInstr *>(FixedInputInstr<1>::Create(alloc, op, type, id, prev, next,
                                                                       std::forward<InstrArg>(arg)));
    }

    std::vector<InstrArg *> GetArgs() override {
        assert(args_.size() == 1);
        return {args_.front()};
    }
};

class TwoInputInstr final : public FixedInputInstr<2> {
public:
    static TwoInputInstr *Create(Allocator *alloc, Opcode op, InstrType type, size_t id,
                                 InstrArg &&arg1, InstrArg &&arg2) {
        return static_cast<TwoInputInstr *>(FixedInputInstr<2>::Create(alloc,
                                                                       op, type, id, std::forward<InstrArg>(arg1),
                                                                       std::forward<InstrArg>(arg2)));
    }

    static TwoInputInstr *Create(Allocator *alloc, Opcode op, InstrType type, size_t id,
                                 InstructionBase *prev, InstructionBase *next,
                                 InstrArg &&arg1, InstrArg &&arg2) {
        return static_cast<TwoInputInstr *>(FixedInputInstr<2>::Create(alloc,
                                                                       op, type, id, prev, next,
                                                                       std::forward<InstrArg>(arg1),
                                                                       std::forward<InstrArg>(arg2)));
    }

    std::vector<InstrArg *> GetArgs() override {
        assert(args_.size() == 2);
        return {args_.at(0), args_.at(1)};
    }
};

class ThreeInputInstr final : public FixedInputInstr<3> {
public:
    static ThreeInputInstr *Create(Allocator *alloc, Opcode op, InstrType type, size_t id,
                                   InstrArg &&arg1, InstrArg &&arg2, InstrArg &&arg3) {
        return ThreeInputInstr::Create(alloc, op, type, id, nullptr, nullptr,
                                       std::forward<InstrArg>(arg1),
                                       std::forward<InstrArg>(arg2),
                                       std::forward<InstrArg>(arg3));
    }

    static ThreeInputInstr *Create(Allocator *alloc, Opcode op, InstrType type, size_t id,
                                   InstructionBase *prev, InstructionBase *next,
                                   InstrArg &&arg1, InstrArg &&arg2, InstrArg &&arg3) {
        return static_cast<ThreeInputInstr *>(FixedInputInstr<3>::Create(alloc,
                                                                         op, type, id, prev, next,
                                                                         std::forward<InstrArg>(arg1),
                                                                         std::forward<InstrArg>(arg2),
                                                                         std::forward<InstrArg>(arg3)));
    }

    std::vector<InstrArg *> GetArgs() override {
        assert(args_.size() == 3);
        return {args_.at(0), args_.at(1), args_.at(2)};
    }
};

}  // namespace compiler

#endif //OPTIMIZER_INSTRUCTION_H
