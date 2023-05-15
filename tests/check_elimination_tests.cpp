#include <gtest/gtest.h>

#include "graph.h"
#include "instruction.h"
#include "check_elimination.h"

namespace compiler::test {

TEST(check_elimination_tests, ExampleZeroCheck) {
    /*
     * Constants and parameters are in bb_start(root) block

     * Current graph:
    u64 main():
        movi.u64 v0, 1           ┌
        addi.u64 v1, v0, 1       |  bb0
        ret.u64 v0               └
     */

    constexpr auto v = InstrArg::Type::v;
    constexpr auto imm = InstrArg::Type::imm;
    constexpr auto U64 = InstrType::U64;

    Allocator alloc;

    // Creating current Graph
    ZeroInputInstr *const1 = ZeroInputInstr::Create(&alloc, Opcode::CONSTANT, U64, {imm, 1});
    BasicBlock bb_start = BasicBlock::MakeBasicBlock({const1});

    OneInputInstr *movi = OneInputInstr::Create(&alloc, Opcode::MOVI, U64, {v, 0}, {imm, 1, const1});
    OneInputInstr *zero_check1 = OneInputInstr::Create(&alloc, Opcode::ZERO_CHECK, U64, {/* acc */}, {v, 0, movi});
    TwoInputInstr *addi = TwoInputInstr::Create(&alloc, Opcode::ADDI, U64, {v, 1}, {v, 0, zero_check1},
                                                {imm, 5, const1});
    OneInputInstr *zero_check2 = OneInputInstr::Create(&alloc, Opcode::ZERO_CHECK, U64, {/* acc */}, {v, 0, movi});
    OneInputInstr *ret = OneInputInstr::Create(&alloc, Opcode::RET, U64, {/* acc */}, {v, 0, zero_check2});
    BasicBlock bb0 = BasicBlock::MakeBasicBlock({movi, zero_check1, addi, zero_check2, ret});

    BasicBlock bb_end = BasicBlock::MakeBasicBlock({});

    // Blocks CFG
    BasicBlock::AddEdge(&bb_start, &bb0);
    BasicBlock::AddEdge(&bb0, &bb_end);

    Graph graph{&alloc, &bb_start, &bb_end, 0};
    graph.SetGraphForBasicBlocks({&bb_start, &bb0, &bb_end});

    // Set users for instructions
    const1->AddUsers({movi});
    movi->AddUsers({zero_check1, zero_check2});
    zero_check1->AddUsers({addi});
    zero_check2->AddUsers({ret});

    // Testing

    // Prove data flow
    ASSERT_EQ(zero_check1->GetUsers().size(), 1);
    ASSERT_EQ(zero_check1->GetUsers().at(0), addi);
    ASSERT_EQ(zero_check1->GetInputs().size(), 1);
    ASSERT_EQ(zero_check1->GetInputs().at(0)->def(), movi);
    ASSERT_TRUE(addi->GetUsers().empty());
    ASSERT_EQ(addi->GetInputs().size(), 2);
    ASSERT_EQ(addi->GetInputs().at(0)->def(), zero_check1);
    ASSERT_EQ(addi->GetInputs().at(1)->def(), const1);
    ASSERT_EQ(zero_check2->GetUsers().size(), 1);
    ASSERT_EQ(zero_check2->GetUsers().at(0), ret);
    ASSERT_EQ(zero_check2->GetInputs().size(), 1);
    ASSERT_EQ(zero_check2->GetInputs().at(0)->def(), movi);
    ASSERT_TRUE(ret->GetUsers().empty());
    ASSERT_EQ(ret->GetInputs().size(), 1);
    ASSERT_EQ(ret->GetInputs().at(0)->def(), zero_check2);

    passes::CheckElimination checkEliminationPass{&graph};
    ASSERT_TRUE(checkEliminationPass.Run());

    // Check instructions order
    auto bb0_instrs = bb0.GetAllInstrs();
    ASSERT_EQ(bb0_instrs.at(0), movi);
    ASSERT_EQ(bb0_instrs.at(0)->GetPrev(), nullptr);
    ASSERT_EQ(bb0_instrs.at(0)->GetNext(), zero_check1);
    ASSERT_EQ(bb0_instrs.at(1), zero_check1);
    ASSERT_EQ(zero_check1->GetOpcode(), Opcode::ZERO_CHECK);
    ASSERT_EQ(bb0_instrs.at(1)->GetPrev(), movi);
    ASSERT_EQ(bb0_instrs.at(1)->GetNext(), addi);
    ASSERT_EQ(bb0_instrs.at(2), addi);
    ASSERT_EQ(bb0_instrs.at(2)->GetPrev(), zero_check1);
    ASSERT_EQ(bb0_instrs.at(2)->GetNext(), zero_check2);
    ASSERT_EQ(bb0_instrs.at(3), zero_check2);
    ASSERT_EQ(zero_check2->GetOpcode(), Opcode::NOP);
    ASSERT_EQ(bb0_instrs.at(3)->GetPrev(), addi);
    ASSERT_EQ(bb0_instrs.at(3)->GetNext(), ret);
    ASSERT_EQ(bb0_instrs.at(4), ret);
    ASSERT_EQ(bb0_instrs.at(4)->GetPrev(), zero_check2);
    ASSERT_EQ(bb0_instrs.at(4)->GetNext(), nullptr);

    // Check data flow
    ASSERT_EQ(zero_check1->GetUsers().size(), 1);
    ASSERT_EQ(zero_check1->GetUsers().at(0), addi);
    ASSERT_EQ(zero_check1->GetInputs().size(), 1);
    ASSERT_EQ(zero_check1->GetInputs().at(0)->def(), movi);
    ASSERT_TRUE(addi->GetUsers().empty());
    ASSERT_EQ(addi->GetInputs().size(), 2);
    ASSERT_EQ(addi->GetInputs().at(0)->def(), zero_check1);
    ASSERT_EQ(addi->GetInputs().at(1)->def(), const1);
    ASSERT_TRUE(zero_check2->GetUsers().empty());
    ASSERT_EQ(zero_check2->GetInputs().size(), 1);
    ASSERT_EQ(zero_check2->GetInputs().at(0), nullptr);
    ASSERT_TRUE(ret->GetUsers().empty());
    ASSERT_EQ(ret->GetInputs().size(), 1);
    ASSERT_EQ(ret->GetInputs().at(0)->def(), zero_check1);
}

TEST(check_elimination_tests, ExampleBoundsCheck) {
    /*
     * Constants and parameters are in bb_start(root) block

     * Current graph:
    u64 main():
        movi.u64 v0, 3                  ┌
        movi.u64 v1, 4                  |
        new_array.u64 v0, v1            |  bb0      // create arr with elems {v0, v1} and load the arr to accumulator
        sta.u64 v3                      |           // store arr from accumulator to v3
        load_array.u64 v4, v3, 1        |           // load arr[1] into v4 register
        load_array.u64 v5, v3, 1        |           // load arr[1] into v5register
        ret.u64 v5                      └
     */

    constexpr auto v = InstrArg::Type::v;
    constexpr auto imm = InstrArg::Type::imm;
    constexpr auto U64 = InstrType::U64;

    Allocator alloc;

    // Creating current Graph
    ZeroInputInstr *const1 = ZeroInputInstr::Create(&alloc, Opcode::CONSTANT, U64, {imm, 1});
    ZeroInputInstr *const3 = ZeroInputInstr::Create(&alloc, Opcode::CONSTANT, U64, {imm, 3});
    ZeroInputInstr *const4 = ZeroInputInstr::Create(&alloc, Opcode::CONSTANT, U64, {imm, 4});
    BasicBlock bb_start = BasicBlock::MakeBasicBlock({const1, const3, const4});

    OneInputInstr *movi3 = OneInputInstr::Create(&alloc, Opcode::MOVI, U64, {v, 0}, {imm, 3, const3});
    OneInputInstr *movi4 = OneInputInstr::Create(&alloc, Opcode::MOVI, U64, {v, 1}, {imm, 4, const4});
    DynamicInputInstr *new_arr = DynamicInputInstr::Create(&alloc, Opcode::NEW_ARRAY, U64, InstrArg{/* acc */},
                                                           InstrArg{v, 0, movi3}, InstrArg{v, 0, movi4});
    OneInputInstr *sta = OneInputInstr::Create(&alloc, Opcode::STA, U64, {v, 3}, {/* acc */});
    OneInputInstr *len_arr = OneInputInstr::Create(&alloc, Opcode::LEN_ARRAY, U64, {v, 1}, {v, 3, sta});
    TwoInputInstr *bounds_check1 = TwoInputInstr::Create(&alloc, Opcode::BOUNDS_CHECK, U64, {/* acc */},
                                                         {v, 3, len_arr}, {imm, 0, const1});
    TwoInputInstr *load_arr1 = TwoInputInstr::Create(&alloc, Opcode::ADDI, U64, {v, 1}, {v, 3, sta},
                                                     {imm, 0, bounds_check1});
    TwoInputInstr *bounds_check2 = TwoInputInstr::Create(&alloc, Opcode::BOUNDS_CHECK, U64, {/* acc */},
                                                         {v, 3, len_arr}, {imm, 1, const1});
    TwoInputInstr *load_arr2 = TwoInputInstr::Create(&alloc, Opcode::ADDI, U64, {v, 1}, {v, 3, sta},
                                                     {imm, 1, bounds_check2});
    OneInputInstr *ret = OneInputInstr::Create(&alloc, Opcode::RET, U64, {/* acc */}, {v, 5, load_arr2});
    BasicBlock bb0 = BasicBlock::MakeBasicBlock(
            {movi3, movi4, new_arr, sta, len_arr, bounds_check1, load_arr1, bounds_check2, load_arr2, ret});

    BasicBlock bb_end = BasicBlock::MakeBasicBlock({});

    // Blocks CFG
    BasicBlock::AddEdge(&bb_start, &bb0);
    BasicBlock::AddEdge(&bb0, &bb_end);

    Graph graph{&alloc, &bb_start, &bb_end, 0};
    graph.SetGraphForBasicBlocks({&bb_start, &bb0, &bb_end});

    // Set users for instructions
    const1->AddUsers({bounds_check2});
    const3->AddUsers({movi3});
    const4->AddUsers({movi4});
    movi3->AddUsers({new_arr});
    movi4->AddUsers({new_arr});
    sta->AddUsers({len_arr, load_arr1, load_arr2});
    len_arr->AddUsers({bounds_check1, bounds_check2});
    bounds_check1->AddUsers({load_arr1});
    bounds_check2->AddUsers({load_arr2});
    load_arr2->AddUsers({ret});

    // Testing

    // Prove data flow
    ASSERT_EQ(len_arr->GetInputs().size(), 1);
    ASSERT_EQ(len_arr->GetInputs().at(0)->def(), sta);
    ASSERT_EQ(len_arr->GetUsers().size(), 2);
    ASSERT_EQ(len_arr->GetUsers().at(0), bounds_check1);
    ASSERT_EQ(len_arr->GetUsers().at(1), bounds_check2);
    ASSERT_EQ(bounds_check1->GetInputs().size(), 2);
    ASSERT_EQ(bounds_check1->GetInputs().at(0)->def(), len_arr);
    ASSERT_EQ(bounds_check1->GetInputs().at(1)->def(), const1);
    ASSERT_EQ(bounds_check1->GetUsers().size(), 1);
    ASSERT_EQ(bounds_check1->GetUsers().at(0), load_arr1);
    ASSERT_EQ(load_arr1->GetInputs().size(), 2);
    ASSERT_EQ(load_arr1->GetInputs().at(0)->def(), sta);
    ASSERT_EQ(load_arr1->GetInputs().at(1)->def(), bounds_check1);
    ASSERT_TRUE(load_arr1->GetUsers().empty());
    ASSERT_EQ(bounds_check2->GetInputs().size(), 2);
    ASSERT_EQ(bounds_check2->GetInputs().at(0)->def(), len_arr);
    ASSERT_EQ(bounds_check2->GetInputs().at(1)->def(), const1);
    ASSERT_EQ(bounds_check2->GetUsers().size(), 1);
    ASSERT_EQ(bounds_check2->GetUsers().at(0), load_arr2);
    ASSERT_EQ(load_arr2->GetInputs().size(), 2);
    ASSERT_EQ(load_arr2->GetInputs().at(0)->def(), sta);
    ASSERT_EQ(load_arr2->GetInputs().at(1)->def(), bounds_check2);
    ASSERT_EQ(load_arr2->GetUsers().size(), 1);
    ASSERT_EQ(load_arr2->GetUsers().at(0), ret);
    ASSERT_EQ(ret->GetInputs().size(), 1);
    ASSERT_EQ(ret->GetInputs().at(0)->def(), load_arr2);
    ASSERT_TRUE(ret->GetUsers().empty());

    passes::CheckElimination checkEliminationPass{&graph};
    ASSERT_TRUE(checkEliminationPass.Run());

    // Check instructions order
    auto bb0_instrs = bb0.GetAllInstrs();
    ASSERT_EQ(bb0_instrs.at(0), movi3);
    ASSERT_EQ(bb0_instrs.at(0)->GetPrev(), nullptr);
    ASSERT_EQ(bb0_instrs.at(0)->GetNext(), movi4);
    ASSERT_EQ(bb0_instrs.at(1), movi4);
    ASSERT_EQ(bb0_instrs.at(1)->GetPrev(), movi3);
    ASSERT_EQ(bb0_instrs.at(1)->GetNext(), new_arr);
    ASSERT_EQ(bb0_instrs.at(2), new_arr);
    ASSERT_EQ(bb0_instrs.at(2)->GetPrev(), movi4);
    ASSERT_EQ(bb0_instrs.at(2)->GetNext(), sta);
    ASSERT_EQ(bb0_instrs.at(3), sta);
    ASSERT_EQ(bb0_instrs.at(3)->GetPrev(), new_arr);
    ASSERT_EQ(bb0_instrs.at(3)->GetNext(), len_arr);
    ASSERT_EQ(bb0_instrs.at(4), len_arr);
    ASSERT_EQ(bb0_instrs.at(4)->GetPrev(), sta);
    ASSERT_EQ(bb0_instrs.at(4)->GetNext(), bounds_check1);
    ASSERT_EQ(bb0_instrs.at(5), bounds_check1);
    ASSERT_EQ(bb0_instrs.at(5)->GetPrev(), len_arr);
    ASSERT_EQ(bb0_instrs.at(5)->GetNext(), load_arr1);
    ASSERT_EQ(bb0_instrs.at(6), load_arr1);
    ASSERT_EQ(bb0_instrs.at(6)->GetPrev(), bounds_check1);
    ASSERT_EQ(bb0_instrs.at(6)->GetNext(), bounds_check2);
    ASSERT_EQ(bb0_instrs.at(7), bounds_check2);
    ASSERT_EQ(bounds_check2->GetOpcode(), Opcode::NOP);
    ASSERT_EQ(bb0_instrs.at(7)->GetPrev(), load_arr1);
    ASSERT_EQ(bb0_instrs.at(7)->GetNext(), load_arr2);
    ASSERT_EQ(bb0_instrs.at(8), load_arr2);
    ASSERT_EQ(bb0_instrs.at(8)->GetPrev(), bounds_check2);
    ASSERT_EQ(bb0_instrs.at(8)->GetNext(), ret);
    ASSERT_EQ(bb0_instrs.at(9), ret);
    ASSERT_EQ(bb0_instrs.at(9)->GetPrev(), load_arr2);
    ASSERT_EQ(bb0_instrs.at(9)->GetNext(), nullptr);

    // Check data flow
    ASSERT_EQ(len_arr->GetInputs().size(), 1);
    ASSERT_EQ(len_arr->GetInputs().at(0)->def(), sta);
    ASSERT_EQ(len_arr->GetUsers().size(), 1);
    ASSERT_EQ(len_arr->GetUsers().at(0), bounds_check1);
    ASSERT_EQ(bounds_check1->GetInputs().size(), 2);
    ASSERT_EQ(bounds_check1->GetInputs().at(0)->def(), len_arr);
    ASSERT_EQ(bounds_check1->GetInputs().at(1)->def(), const1);
    ASSERT_EQ(bounds_check1->GetUsers().size(), 1);
    ASSERT_EQ(bounds_check1->GetUsers().at(0), load_arr1);
    ASSERT_EQ(load_arr1->GetInputs().size(), 2);
    ASSERT_EQ(load_arr1->GetInputs().at(0)->def(), sta);
    ASSERT_EQ(load_arr1->GetInputs().at(1)->def(), bounds_check1);
    ASSERT_TRUE(load_arr1->GetUsers().empty());
    ASSERT_EQ(bounds_check2->GetInputs().size(), 2);
    ASSERT_EQ(bounds_check2->GetInputs().at(0), nullptr);
    ASSERT_EQ(bounds_check2->GetInputs().at(1), nullptr);
    ASSERT_TRUE(bounds_check2->GetUsers().empty());
    ASSERT_EQ(load_arr2->GetInputs().size(), 2);
    ASSERT_EQ(load_arr2->GetInputs().at(0)->def(), sta);
    ASSERT_EQ(load_arr2->GetInputs().at(1)->def(), bounds_check1);
    ASSERT_EQ(load_arr2->GetUsers().size(), 1);
    ASSERT_EQ(load_arr2->GetUsers().at(0), ret);
    ASSERT_EQ(ret->GetInputs().size(), 1);
    ASSERT_EQ(ret->GetInputs().at(0)->def(), load_arr2);
    ASSERT_TRUE(ret->GetUsers().empty());
}

}  // namespace compiler::test

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
