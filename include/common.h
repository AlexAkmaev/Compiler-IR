#ifndef OPTIMIZER_COMMON_H
#define OPTIMIZER_COMMON_H

#include <cstdint>
#include <iostream>
#include <type_traits>
#include <cassert>

#include "allocator.h"

namespace compiler {

class InstrArg;

class DynamicInputInstr;
class ZeroInputInstr;
class OneInputInstr;
class TwoInputInstr;
class ThreeInputInstr;

class BasicBlock;

using Allocator = storage::Allocator<BasicBlock, InstrArg, DynamicInputInstr, ZeroInputInstr,
        OneInputInstr, TwoInputInstr, ThreeInputInstr>;

class InstructionBase;

class Graph;

using vreg_t = uint16_t;
using BlocksVector = std::vector<BasicBlock *>;
using InsnsVec = std::vector<InstructionBase *>;

enum class Opcode : uint8_t {
    NONE,
    PHI,
    CAST,
    CONSTANT,
    PARAMETER,
    ADDI,
    ADD,
    SUBI,
    SUB,
    MULI,
    MUL,
    MOVI,
    MOV,
    CMP,
    JA,
    JAE,
    JG,
    JGE,
    JB,
    JBE,
    JL,
    JLE,
    JE,
    JNE,
    JO,
    JMP,
    CALL,
    RET,
    RET_VOID,
    THROW
};

enum class InstrType : uint8_t {
    U8,
    U16,
    U32,
    U64,
    I8,
    I16,
    I32,
    I64
};

class InstrArg {
public:
    /**
     * The Type enum represents the type of argument that is constructing.
     * For `a` and `v` types: `num` means num of a register.
     * For `imm` type: `num` means the value of immediate.
     * For `id` type: `num` means the instruction id that corresponds to the target or def instr; or
     *              : `num` means the graph id that corresponds to the callee graph
     */
    enum Type {
        acc, a, v, imm, id, callee_graph, none  // acc - accumulator, a - func parameter, v - virtual reg
    };

    InstrArg(Type type) : type_(type) {
        if (type_ != Type::acc) {
            std::cerr << "Warning! Must be accumulator type." << std::endl;
        }
    }

    InstrArg(Type type, vreg_t num, InstructionBase *ref = nullptr);  // ref = target or def

    InstrArg(vreg_t num, Graph *graph) : type_(Type::callee_graph), num_(num), callee_(graph) {}

    [[nodiscard]] vreg_t num() const {
        return num_;
    }

    [[nodiscard]] Type type() const {
        return type_;
    }

    [[nodiscard]] InstructionBase *target() const {
        assert(type_ == Type::id);
        return ref_;
    }

    [[nodiscard]] Graph *callee() const {
        assert(type_ == Type::callee_graph);
        return callee_;
    }

    [[nodiscard]] InstructionBase *def() const {
        assert(type_ == Type::a || type_ == Type::v);
        return ref_;
    }

    void SetDef(InstructionBase *def) {
        ref_ = def;
    }

    bool operator==(const InstrArg &arg) const {
        return type_ == arg.type_ && num_ == arg.num_ && ref_ == arg.ref_ && callee_ == arg.callee_;
    }

private:
    Type type_;
    size_t num_{};  // number of virtual register, or the value of immediate, or the Instruction id of target
    InstructionBase *ref_{nullptr};  // target instruction to which the jump will happen; or a definition for this input
    Graph *callee_{nullptr};  // callee graph in case if instruction itself is call
};

// The specialized hash function for `unordered_map` keys
struct hash_instr_arg {
    std::size_t operator()(const InstrArg &arg) const {
        std::size_t h1 = std::hash<vreg_t>()(arg.num());
        std::size_t h2 = std::hash<size_t>()(arg.type());
        return h1 ^ h2;
    }
};

}  // namespace compiler

#endif //OPTIMIZER_COMMON_H
