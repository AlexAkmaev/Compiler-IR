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

    [[nodiscard]] Opcode GetOpcode() const noexcept {
        return op_;
    }

    void SetId(size_t id) noexcept {
        id_ = id;
    }

    [[nodiscard]] size_t GetId() const noexcept {
        return id_;
    }

    void SetType(InstrType type) noexcept {
        type_ = type;
    }

    [[nodiscard]] InstrType GetType() const noexcept {
        return type_;
    }

    void AddUsers(std::initializer_list<InstructionBase *> uses) {
        users_.insert(users_.end(), uses.begin(), uses.end());
    }

    void AddUsers(std::vector<InstructionBase *> uses) {
        users_.insert(users_.end(), uses.begin(), uses.end());
    }

    [[nodiscard]] std::vector<InstructionBase *> GetUsers() const noexcept {
        return users_;
    }

    void RemoveUser(const InstructionBase *instr) {
        for (auto it = users_.begin(); it != users_.end(); ++it) {
            if (*it == instr) {
                users_.erase(it);
            }
        }
    }

    // Replace user that point to this instruction by given instruction.
    void ReplaceUserForInputs(InstructionBase *new_user) const {
        assert(new_user != nullptr && new_user != this);
        for (auto *input_arg: GetInputs()) {
            auto *input_instr = input_arg->def();
            const auto &users = input_instr->GetUsers();
            input_instr->RemoveUser(this);
            input_instr->AddUsers({new_user});
        }
    }

    // Replace inputs that point to this instruction by given instruction.
    void ReplaceInputForUsers(InstructionBase *new_input) const {
        assert(new_input != nullptr && new_input != this);
        for (auto *user: users_) {
            for (auto *input: user->GetInputs()) {
                if (input->def() == this) {
                    input->SetDef(new_input);
                }
            }
        }
    }

    void SetPrev(InstructionBase *prev) noexcept {
        prev_ = prev;
    }

    [[nodiscard]] InstructionBase *GetPrev() const {
        return prev_;
    }

    void SetNext(InstructionBase *next) noexcept {
        next_ = next;
    }

    [[nodiscard]] InstructionBase *GetNext() const noexcept {
        return next_;
    }

    void SetBasicBlock(BasicBlock *bb) noexcept {
        bb_ = bb;
    }

    [[nodiscard]] BasicBlock *GetBasicBlock() const noexcept {
        return bb_;
    }

    [[nodiscard]] bool IsControlFlow() const noexcept {
        return op_ >= Opcode::JA;
    }

    [[nodiscard]] bool IsJump() const noexcept {
        return op_ == Opcode::JMP;
    }

    [[nodiscard]] bool IsConditionalBranch() const noexcept {
        return op_ >= Opcode::JA && op_ <= Opcode::JNE;
    }

    [[nodiscard]] bool IsReturn() const noexcept {
        return op_ == Opcode::RET;
    }

    [[nodiscard]] bool IsCall() const {
        return op_ == Opcode::CALL;
    }

    [[nodiscard]] bool IsConstant() const {
        return op_ == Opcode::CONSTANT;
    }

    bool IsParameter() {
        return op_ == Opcode::PARAMETER;
    }

    [[nodiscard]] bool IsTarget() const {
        return is_target_;
    }

    void SetIsTarget(bool is_tgt) noexcept {
        is_target_ = is_tgt;
    }

    [[nodiscard]] InstrArg *GetDst() const noexcept {
        assert(dst_ != nullptr && "dst location must not be nullptr");
        return dst_;
    }

    [[nodiscard]] virtual bool HasInputs() const = 0;

    [[nodiscard]] virtual std::vector<InstrArg *> GetInputs() const = 0;

    virtual ~InstructionBase() = default;

protected:
    explicit InstructionBase(Opcode op);

    InstructionBase(Opcode op, InstrType type, InstrArg *dst);

    InstructionBase(Opcode op, InstrType type, InstructionBase *prev, InstructionBase *next, InstrArg *dst);


    Opcode op_{Opcode::NONE};
    InstrType type_{InstrType::I32};

    size_t id_{static_cast<size_t>(-1)};

    InstructionBase *prev_{nullptr};
    InstructionBase *next_{nullptr};

    std::vector<InstructionBase *> users_;
    InstrArg *dst_{nullptr};

    BasicBlock *bb_{nullptr};

private:
    bool is_target_{false};  // is this instruction is target to some jump
};

class DynamicInputInstr final : public InstructionBase {
public:
    template<typename... Args, std::enable_if_t<(std::is_same_v<Args, InstrArg> && ...), bool> = true>
    static DynamicInputInstr *
    Create(Allocator *alloc, Opcode op, InstrType type, InstrArg &&dst, Args &&... inputs) {
        return DynamicInputInstr::Create(alloc, op, type, nullptr, nullptr,
                                         std::forward<InstrArg>(dst), std::forward<InstrArg>(inputs)...);
    }

    template<typename... Args, std::enable_if_t<(std::is_same_v<Args, InstrArg> && ...), bool> = true>
    static DynamicInputInstr *
    Create(Allocator *alloc, Opcode op, InstrType type, InstructionBase *prev, InstructionBase *next,
           InstrArg &&dst, Args &&... inputs) {
        return alloc->New<DynamicInputInstr>(
                DynamicInputInstr(op, type, prev, next,
                                  alloc->New<InstrArg>(std::forward<InstrArg>(dst)),
                                  alloc->NewPool<InstrArg>(std::forward<InstrArg>(inputs)...)));
    }

    void AddArgDefs(std::initializer_list<InstrArg *> inputs) {
        inputs_.insert(inputs_.end(), inputs.begin(), inputs.end());
    }

    [[nodiscard]] bool HasInputs() const override {
        return !inputs_.empty();
    }

    [[nodiscard]] std::vector<InstrArg *> GetInputs() const override {
        return inputs_;
    }

    void SetInputs(std::vector<InstrArg *> &&inputs) {
        inputs_ = std::move(inputs);
    }

private:
    DynamicInputInstr(Opcode op, InstrType type, InstrArg *dst, std::vector<InstrArg *> &&inputs)
            : InstructionBase(op, type, dst), inputs_(std::move(inputs)) {}

    DynamicInputInstr(Opcode op, InstrType type, InstructionBase *prev, InstructionBase *next, InstrArg *dst,
                      std::vector<InstrArg *> &&inputs)
            : InstructionBase(op, type, prev, next, dst), inputs_(std::move(inputs)) {}

    std::vector<InstrArg *> inputs_;
};

class ZeroInputInstr final : public InstructionBase {
public:
    static ZeroInputInstr *Create(Allocator *alloc, Opcode op, InstrType type, InstrArg &&dst) {
        return ZeroInputInstr::Create(alloc, op, type, nullptr, nullptr, std::forward<InstrArg>(dst));
    }

    static ZeroInputInstr *
    Create(Allocator *alloc, Opcode op, InstrType type, InstructionBase *prev, InstructionBase *next,
           InstrArg &&dst) {
        return alloc->New<ZeroInputInstr>(ZeroInputInstr(op, type, prev, next,
                                                         alloc->New<InstrArg>(std::forward<InstrArg>(dst))));
    }

    bool HasInputs() const override {
        return false;
    }

    [[nodiscard]] std::vector<InstrArg *> GetInputs() const override {
        return {};
    }

private:
    ZeroInputInstr(Opcode op, InstrType type, InstrArg *dst) : InstructionBase(op, type, dst) {}

    ZeroInputInstr(Opcode op, InstrType type, InstructionBase *prev, InstructionBase *next, InstrArg *dst)
            : InstructionBase(op, type, prev, next, dst) {}
};

/**
 *  Cannot be instantiated directly, use derived classes.
 *  N - amount of inputs
 */
template<size_t N>
class FixedInputInstr : public InstructionBase {
public:
    virtual ~FixedInputInstr() = default;

protected:
    template<class Derived, typename... Args, std::enable_if_t<(std::is_same_v<Args, InstrArg> && ...), bool> = true>
    static Derived *Create(Allocator *alloc, Opcode op, InstrType type, InstrArg &&dst, Args &&... inputs) {
        return FixedInputInstr::Create<Derived>(alloc, op, type, nullptr, nullptr,
                                                std::forward<InstrArg>(dst),
                                                std::forward<InstrArg>(inputs)...);
    }

    template<class Derived, typename... Args, std::enable_if_t<(std::is_same_v<Args, InstrArg> && ...), bool> = true>
    static Derived *Create(Allocator *alloc, Opcode op, InstrType type,
                           InstructionBase *prev, InstructionBase *next, InstrArg &&dst, Args &&... inputs) {
        static_assert(sizeof...(Args) == N, "Number of arguments does not match array size");
        auto instr = Derived(op, type, prev, next, alloc->New<InstrArg>(std::forward<InstrArg>(dst)));
        instr.inputs_ = alloc->NewPoolAsArray<InstrArg>(std::forward<InstrArg>(inputs)...);
        return alloc->New<Derived>(std::move(instr));
    }

    [[nodiscard]] bool HasInputs() const override {
        static_assert(N > 0 && "For N = 0 use ZeroInputInstr");
        return true;
    }

    [[nodiscard]] std::vector<InstrArg *> GetInputs() const override {
        assert(0 && "Cannot be used for this class");
        return {};
    }

    void SetInputs(std::array<InstrArg *, N> &&inputs) {
        inputs_ = std::move(inputs);
    }

protected:
    std::array<InstrArg *, N> inputs_;

protected:
    FixedInputInstr(Opcode op, InstrType type, InstrArg *dst) : InstructionBase(op, type, dst) {}

    FixedInputInstr(Opcode op, InstrType type, InstructionBase *prev, InstructionBase *next, InstrArg *dst)
            : InstructionBase(op, type, prev, next, dst) {}

};

class OneInputInstr final : public FixedInputInstr<1> {
public:
    static OneInputInstr *
    Create(Allocator *alloc, Opcode op, InstrType type, InstrArg &&dst, InstrArg &&arg) {
        return OneInputInstr::Create(alloc, op, type, nullptr, nullptr,
                                     std::forward<InstrArg>(dst), std::forward<InstrArg>(arg));
    }

    static OneInputInstr *Create(Allocator *alloc, Opcode op, InstrType type,
                                 InstructionBase *prev, InstructionBase *next, InstrArg &&dst,
                                 InstrArg &&arg) {
        return FixedInputInstr::Create<OneInputInstr>(alloc, op, type, prev, next,
                                                      std::forward<InstrArg>(dst),
                                                      std::forward<InstrArg>(arg));
    }

    [[nodiscard]] std::vector<InstrArg *> GetInputs() const override {
        assert(inputs_.size() == 1);
        return {inputs_.front()};
    }

public:
    // Not recommended, better use Create methods
    OneInputInstr(Opcode op, InstrType type, InstructionBase *prev, InstructionBase *next, InstrArg *dst)
            : FixedInputInstr<1>(op, type, prev, next, dst) {}
};

class TwoInputInstr final : public FixedInputInstr<2> {
public:
    static TwoInputInstr *Create(Allocator *alloc, Opcode op, InstrType type, InstrArg &&dst,
                                 InstrArg &&arg1, InstrArg &&arg2) {
        return TwoInputInstr::Create(alloc, op, type, nullptr, nullptr,
                                     std::forward<InstrArg>(dst),
                                     std::forward<InstrArg>(arg1),
                                     std::forward<InstrArg>(arg2));
    }

    static TwoInputInstr *Create(Allocator *alloc, Opcode op, InstrType type,
                                 InstructionBase *prev, InstructionBase *next, InstrArg &&dst,
                                 InstrArg &&arg1, InstrArg &&arg2) {
        return FixedInputInstr::Create<TwoInputInstr>(alloc, op, type, prev, next,
                                                      std::forward<InstrArg>(dst),
                                                      std::forward<InstrArg>(arg1),
                                                      std::forward<InstrArg>(arg2));
    }

    [[nodiscard]] std::vector<InstrArg *> GetInputs() const override {
        assert(inputs_.size() == 2);
        return {inputs_.at(0), inputs_.at(1)};
    }

public:
    // Not recommended, better use Create methods
    TwoInputInstr(Opcode op, InstrType type, InstructionBase *prev, InstructionBase *next, InstrArg *dst)
            : FixedInputInstr<2>(op, type, prev, next, dst) {}
};

class ThreeInputInstr final : public FixedInputInstr<3> {
public:
    static ThreeInputInstr *Create(Allocator *alloc, Opcode op, InstrType type, InstrArg &&dst,
                                   InstrArg &&arg1, InstrArg &&arg2, InstrArg &&arg3) {
        return ThreeInputInstr::Create(alloc, op, type, nullptr, nullptr,
                                       std::forward<InstrArg>(dst),
                                       std::forward<InstrArg>(arg1),
                                       std::forward<InstrArg>(arg2),
                                       std::forward<InstrArg>(arg3));
    }

    static ThreeInputInstr *Create(Allocator *alloc, Opcode op, InstrType type,
                                   InstructionBase *prev, InstructionBase *next, InstrArg &&dst,
                                   InstrArg &&arg1, InstrArg &&arg2, InstrArg &&arg3) {
        return FixedInputInstr::Create<ThreeInputInstr>(alloc, op, type, prev, next,
                                                        std::forward<InstrArg>(dst),
                                                        std::forward<InstrArg>(arg1),
                                                        std::forward<InstrArg>(arg2),
                                                        std::forward<InstrArg>(arg3));
    }

    [[nodiscard]] std::vector<InstrArg *> GetInputs() const override {
        assert(inputs_.size() == 3);
        return {inputs_.at(0), inputs_.at(1), inputs_.at(2)};
    }

public:
    // Not recommended, better use Create methods
    ThreeInputInstr(Opcode op, InstrType type, InstructionBase *prev, InstructionBase *next, InstrArg *dst)
            : FixedInputInstr<3>(op, type, prev, next, dst) {}
};

}  // namespace compiler

#endif //OPTIMIZER_INSTRUCTION_H
