#ifndef OPTIMIZER_COMMON_H
#define OPTIMIZER_COMMON_H

#include <cstdint>
#include <iostream>
#include <type_traits>

namespace compiler {

using vreg_t = uint16_t;

enum class Opcode : uint8_t {
    NONE,
    PHI,
    CAST,
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
    RET
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
    enum Type {
        a, v, imm, id  // a - func parameter, v - virtual reg
    };

    InstrArg(Type type, vreg_t num) : num_(num), type_(type) {}

    [[nodiscard]] vreg_t num() const {
        return num_;
    }

    [[nodiscard]] Type type() const {
        return type_;
    }

    bool operator==(const InstrArg &arg) const {
        return type_ == arg.type_ && num_ == arg.num_;
    }

private:
    size_t num_;
    Type type_;
};

// The specialized hash function for `unordered_map` keys
struct hash_instr_arg
{
    std::size_t operator() (const InstrArg &arg) const
    {
        std::size_t h1 = std::hash<vreg_t>()(arg.num());
        std::size_t h2 = std::hash<size_t>()(arg.type());
        return h1 ^ h2;
    }
};

}  // namespace compiler

#endif //OPTIMIZER_COMMON_H
