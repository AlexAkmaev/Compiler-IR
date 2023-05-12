#include <gtest/gtest.h>

#include "graph.h"
#include "pass.h"
#include "instruction.h"

namespace compiler::test {

TEST(basic_tests, example) {
    /*
    u64 fact (u32 a0):
        // constants and parameters are in bb_start block
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

    constexpr auto acc = InstrArg::Type::acc;
    constexpr auto a = InstrArg::Type::a;
    constexpr auto v = InstrArg::Type::v;
    constexpr auto imm = InstrArg::Type::imm;
    constexpr auto id = InstrArg::Type::id;
    constexpr auto U64 = InstrType::U64;

    Allocator alloc;

    // Creating Graph
    ZeroInputInstr *param0 = ZeroInputInstr::Create(&alloc, Opcode::PARAMETER, U64, {a, 0});
    ZeroInputInstr *const1 = ZeroInputInstr::Create(&alloc, Opcode::CONSTANT, U64, {imm, 1});
    ZeroInputInstr *const2 = ZeroInputInstr::Create(&alloc, Opcode::CONSTANT, U64, {imm, 2});
    BasicBlock bb_start = BasicBlock::MakeBasicBlock({param0, const1, const2});

    TwoInputInstr *addi = TwoInputInstr::Create(&alloc, Opcode::ADDI, U64, {v, 1}, {v, 1}, {imm, 1, const1});
    TwoInputInstr *mul = TwoInputInstr::Create(&alloc, Opcode::MUL, U64, {v, 0}, {v, 0}, {v, 1});
    ZeroInputInstr *ret = ZeroInputInstr::Create(&alloc, Opcode::RET, U64, {v, 0});  // done label

    OneInputInstr *movi1 = OneInputInstr::Create(&alloc, Opcode::MOVI, U64, {v, 0}, {imm, 1, const1});
    OneInputInstr *movi2 = OneInputInstr::Create(&alloc, Opcode::MOVI, U64, {v, 1}, {imm, 2, const2});
    OneInputInstr *u32tou64 = OneInputInstr::Create(&alloc, Opcode::CAST, U64, {v, 2}, {a, 0, param0});
    BasicBlock bb0 = BasicBlock::MakeBasicBlock({movi1, movi2, u32tou64});

    DynamicInputInstr *phi1 = DynamicInputInstr::Create(&alloc, Opcode::PHI, U64, InstrArg{v, 1, addi},
                                                        InstrArg{v, 1, movi1});
    TwoInputInstr *cmp = TwoInputInstr::Create(&alloc, Opcode::CMP, U64, {acc}, {v, 1, phi1},
                                               {v, 2, u32tou64});  // loop label
    OneInputInstr *ja = OneInputInstr::Create(&alloc, Opcode::JA, U64, {id, 9, ret},
                                              {acc});  // id = 9 ("done" label)
    BasicBlock bb1 = BasicBlock::MakeBasicBlock({phi1, cmp, ja});

    DynamicInputInstr *phi2 = DynamicInputInstr::Create(&alloc, Opcode::PHI, U64, {acc}, InstrArg{v, 0, mul},
                                                        InstrArg{v, 0, movi1});
    ZeroInputInstr *jmp = ZeroInputInstr::Create(&alloc, Opcode::JMP, U64, {id, 4, cmp});  // id = 4 ("loop" label)
    BasicBlock bb2 = BasicBlock::MakeBasicBlock({phi2, mul, addi, jmp});

    BasicBlock bb3 = BasicBlock::MakeBasicBlock({ret});

    BasicBlock bb_end = BasicBlock::MakeBasicBlock({});

    // Phi funcs
    phi1->AddUsers({cmp, addi});

    phi2->AddUsers({mul, ret});

    // Blocks CFG
    BasicBlock::AddEdge(&bb_start, &bb0);

    BasicBlock::AddEdge(&bb0, &bb1);

    BasicBlock::AddEdge(&bb1, &bb2);
    BasicBlock::AddEdge(&bb1, &bb3);

    BasicBlock::AddEdge(&bb2, &bb1);

    BasicBlock::AddEdge(&bb3, &bb_end);

    Graph graph{&alloc, &bb_start, &bb_end, 1};
    graph.SetGraphForBasicBlocks({&bb_start, &bb0, &bb1, &bb2, &bb3, &bb_end});

    // Testing

    ASSERT_EQ(movi1->GetPrev(), nullptr);
    ASSERT_EQ(movi1->GetNext(), movi2);
    ASSERT_EQ(movi2->GetPrev(), movi1);
    ASSERT_EQ(movi2->GetNext(), u32tou64);
    ASSERT_EQ(u32tou64->GetPrev(), movi2);
    ASSERT_EQ(u32tou64->GetNext(), nullptr);
    ASSERT_EQ(phi1->GetPrev(), nullptr);
    ASSERT_EQ(cmp->GetPrev(), phi1);
    ASSERT_EQ(cmp->GetNext(), ja);
    ASSERT_EQ(ja->GetPrev(), cmp);
    ASSERT_EQ(ja->GetNext(), nullptr);
    ASSERT_EQ(phi2->GetPrev(), nullptr);
    ASSERT_EQ(mul->GetPrev(), phi2);
    ASSERT_EQ(mul->GetNext(), addi);
    ASSERT_EQ(addi->GetPrev(), mul);
    ASSERT_EQ(addi->GetNext(), jmp);
    ASSERT_EQ(jmp->GetPrev(), addi);
    ASSERT_EQ(jmp->GetNext(), nullptr);

    ASSERT_TRUE(bb_start.GetPreds().empty());
    ASSERT_EQ(bb_start.GetSuccs().size(), 1);
    ASSERT_EQ(bb0.GetPreds().size(), 1);
    ASSERT_EQ(bb0.GetSuccs().size(), 1);
    ASSERT_EQ(bb1.GetPreds().size(), 2);
    ASSERT_EQ(bb1.GetSuccs().size(), 2);
    ASSERT_EQ(bb2.GetPreds().size(), 1);
    ASSERT_EQ(bb2.GetSuccs().size(), 1);
    ASSERT_EQ(bb3.GetPreds().size(), 1);
    ASSERT_EQ(bb3.GetSuccs().size(), 1);
    ASSERT_EQ(bb_end.GetPreds().size(), 1);
    ASSERT_TRUE(bb_end.GetSuccs().empty());

    ASSERT_EQ(graph.GetRoot(), &bb_start);
    ASSERT_EQ(graph.GetEnd(), &bb_end);
}

}  // namespace compiler::test

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
