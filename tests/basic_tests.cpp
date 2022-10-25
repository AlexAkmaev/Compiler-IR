#include <gtest/gtest.h>

#include "Graph.h"

namespace compiler::test {

TEST(basic_tests, example) {
    /*
    u64 fact (u32 a0):
        movi.u64 v0, 1          ┌
        movi.u64 v1, 2          |  bb0
        u32tou64 v2, a0         └
    loop:                       ┌
        cmp.u64 v1, v2          |  bb1
        ja done                 └
        mul.u64 v0, v0, v1      ┌
        addi.u64 v1, v1, 1      |  bb2
        jmp loop                └
    done:                       ┌  bb3
        ret.u64 v0              └
     */

    constexpr auto a = InstrArg::Type::a;
    constexpr auto v = InstrArg::Type::v;
    constexpr auto imm = InstrArg::Type::imm;
    constexpr auto id = InstrArg::Type::id;
    constexpr auto U64 = InstrType::U64;
    
    // Creating Graph

    Instruction movi1{Opcode::MOVI, U64, {{v, 0}, {imm, 1}}, 0};
    Instruction movi2{Opcode::MOVI, U64, {{v, 1}, {imm, 2}}, 1};
    Instruction u32tou64{Opcode::CAST, U64, {{v, 2}, {a, 0}}, 2};
    BasicBlock bb0 = BasicBlock::MakeBasicBlock({&movi1, &movi2, &u32tou64});

    PhiInstruction phi1 = PhiInstruction::CreatePhi(U64, 3);
    Instruction cmp{Opcode::CMP, U64, {{v, 1}, {v, 2}}, 4};
    Instruction ja{Opcode::JA, U64, {{id, 0}}, 5};  // id = 0 ("done" label)
    BasicBlock bb1 = BasicBlock::MakeBasicBlock({&phi1, &cmp, &ja});

    PhiInstruction phi2 = PhiInstruction::CreatePhi(U64, 6);
    Instruction mul{Opcode::MUL, U64, {{v, 0}, {v, 0}, {v, 1}}, 7};
    Instruction addi{Opcode::ADDI, U64, {{v, 1}, {v, 1}, {imm, 1}}, 8};
    Instruction jmp{Opcode::JMP, U64, {{id, 1}}, 7};  // id = 1 ("loop" label)
    BasicBlock bb2 = BasicBlock::MakeBasicBlock({&mul, &addi, &jmp});

    Instruction ret{Opcode::RET, U64, {{v, 0}}, 9};
    BasicBlock bb3 = BasicBlock::MakeBasicBlock({&ret});

    BasicBlock bb_end = BasicBlock::MakeBasicBlock({});

    // Phi funcs
    phi1.AddDefs({&movi2, &addi});
    phi1.AddUses({&cmp, &addi});

    phi2.AddDefs({&movi1, &mul});
    phi2.AddUses({&mul, &ret});

    // Blocks CFG
    bb0.AddToSuccs({&bb1});

    bb1.AddToPreds({&bb0, &bb2});
    bb1.AddToSuccs({&bb2, &bb3});

    bb2.AddToPreds({&bb1});
    bb2.AddToSuccs({&bb1});

    bb3.AddToPreds({&bb1});
    bb3.AddToSuccs({&bb_end});

    bb_end.AddToPreds({&bb3});

    Graph graph{&bb0, &bb_end, 1};
    graph.SetGraphForBasicBlocks({&bb0, &bb1, &bb2, &bb3, &bb_end});
    
    // Testing

    ASSERT_EQ(movi1.GetPrev(), nullptr);
    ASSERT_EQ(movi1.GetNext(), &movi2);
    ASSERT_EQ(movi2.GetPrev(), &movi1);
    ASSERT_EQ(movi2.GetNext(), &u32tou64);
    ASSERT_EQ(u32tou64.GetPrev(), &movi2);
    ASSERT_EQ(u32tou64.GetNext(), nullptr);
    ASSERT_EQ(cmp.GetPrev(), nullptr);
    ASSERT_EQ(cmp.GetNext(), &ja);
    ASSERT_EQ(ja.GetPrev(), &cmp);
    ASSERT_EQ(ja.GetNext(), nullptr);
    ASSERT_EQ(mul.GetPrev(), nullptr);
    ASSERT_EQ(mul.GetNext(), &addi);
    ASSERT_EQ(addi.GetPrev(), &mul);
    ASSERT_EQ(addi.GetNext(), &jmp);
    ASSERT_EQ(jmp.GetPrev(), &addi);
    ASSERT_EQ(jmp.GetNext(), nullptr);

    ASSERT_TRUE(bb0.GetPreds().empty());
    ASSERT_EQ(bb0.GetSuccs().size(), 1);
    ASSERT_EQ(bb1.GetPreds().size(), 2);
    ASSERT_EQ(bb1.GetSuccs().size(), 2);
    ASSERT_EQ(bb2.GetPreds().size(), 1);
    ASSERT_EQ(bb2.GetSuccs().size(), 1);
    ASSERT_EQ(bb3.GetPreds().size(), 1);
    ASSERT_EQ(bb3.GetSuccs().size(), 1);
    ASSERT_EQ(bb_end.GetPreds().size(), 1);
    ASSERT_TRUE(bb_end.GetSuccs().empty());

    ASSERT_EQ(graph.GetRoot(), &bb0);
    ASSERT_EQ(graph.GetEnd(), &bb_end);
}

}  // namespace compiler::test

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
