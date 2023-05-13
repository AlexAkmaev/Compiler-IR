#include <gtest/gtest.h>

#include "graph.h"
#include "instruction.h"
#include "inlining.h"

namespace compiler::test {

TEST(inlining_tests, Example1) {
    /*
     * Constants and parameters are in bb_start(root) block

     * Current graph:
    u64 main():
        movi.u64 v0, 1           ┌
        movi.u64 v1, 5           |
        call.u64 foo, v0, v1     |  bb0
        sta.u64 v3               |
        ret.u64 v3               └

     * Inlined graph:
    u64 foo(u64 a0, u64 a1):
        cmp.u64 a0, a1          ┌  bb0
        jb sub_1_0              └
        sub.u64 v0, a0, a1      ┌  bb1
        jmp done                └
    sub_1_0:                    ┌  bb2
        sub.u64 v0, a1, a0      └
    done:                       ┌
        addi.u64 v0, v0, 5      |  bb3
        ret.u64 v0              └
     */

    constexpr auto a = InstrArg::Type::a;
    constexpr auto v = InstrArg::Type::v;
    constexpr auto imm = InstrArg::Type::imm;
    constexpr auto id = InstrArg::Type::id;
    constexpr auto U64 = InstrType::U64;

    Allocator alloc;

    // Creating current Graph
    ZeroInputInstr *const1 = ZeroInputInstr::Create(&alloc, Opcode::CONSTANT, U64, {imm, 1});
    ZeroInputInstr *const5 = ZeroInputInstr::Create(&alloc, Opcode::CONSTANT, U64, {imm, 5});
    BasicBlock curr_bb_start = BasicBlock::MakeBasicBlock({const1, const5});

    OneInputInstr *movi1 = OneInputInstr::Create(&alloc, Opcode::MOVI, U64, {v, 0}, {imm, 1, const1});
    OneInputInstr *movi2 = OneInputInstr::Create(&alloc, Opcode::MOVI, U64, {v, 1}, {imm, 5, const5});
    DynamicInputInstr *call = DynamicInputInstr::Create(&alloc, Opcode::CALL, U64, {/* acc */});
    OneInputInstr *sta = OneInputInstr::Create(&alloc, Opcode::STA, U64, {v, 3}, {/* acc */});
    OneInputInstr *ret = OneInputInstr::Create(&alloc, Opcode::RET, U64, {/* acc */}, {v, 3, sta});
    BasicBlock curr_bb0 = BasicBlock::MakeBasicBlock({movi1, movi2, call, sta, ret});

    BasicBlock curr_bb_end = BasicBlock::MakeBasicBlock({});

    // Blocks CFG
    BasicBlock::AddEdge(&curr_bb_start, &curr_bb0);
    BasicBlock::AddEdge(&curr_bb0, &curr_bb_end);

    Graph curr_graph{&alloc, &curr_bb_start, &curr_bb_end, 0};
    curr_graph.SetGraphForBasicBlocks({&curr_bb_start, &curr_bb0, &curr_bb_end});


    // Creating inlined Graph
    ZeroInputInstr *param0 = ZeroInputInstr::Create(&alloc, Opcode::PARAMETER, U64, {a, 0});
    ZeroInputInstr *param1 = ZeroInputInstr::Create(&alloc, Opcode::PARAMETER, U64, {a, 1});
    ZeroInputInstr *inl_const5 = ZeroInputInstr::Create(&alloc, Opcode::CONSTANT, U64, {imm, 5});
    BasicBlock bb_start = BasicBlock::MakeBasicBlock({param0, param1, inl_const5});

    TwoInputInstr *sub_versa = TwoInputInstr::Create(&alloc, Opcode::SUB, U64, {v, 0}, {a, 1, param1}, {a, 0, param0});
    BasicBlock bb2 = BasicBlock::MakeBasicBlock({sub_versa});

    TwoInputInstr *cmp = TwoInputInstr::Create(&alloc, Opcode::CMP, U64, {/* acc */}, {a, 0, param0}, {a, 1, param1});
    OneInputInstr *jb = OneInputInstr::Create(&alloc, Opcode::JB, U64, {id, 0, sub_versa}, {/* acc */});
    BasicBlock bb0 = BasicBlock::MakeBasicBlock({cmp, jb});

    DynamicInputInstr *phi = DynamicInputInstr::Create(&alloc, Opcode::PHI, U64, InstrArg{v, 0});

    TwoInputInstr *sub = TwoInputInstr::Create(&alloc, Opcode::SUB, U64, {v, 0}, {a, 0, param0}, {a, 1, param1});
    ZeroInputInstr *jmp = ZeroInputInstr::Create(&alloc, Opcode::JMP, U64, {id, 0, phi});  // target is "done" label
    BasicBlock bb1 = BasicBlock::MakeBasicBlock({sub, jmp});

    TwoInputInstr *addi = TwoInputInstr::Create(&alloc, Opcode::ADDI, U64, {v, 0}, {v, 0, phi}, {imm, 5, inl_const5});
    OneInputInstr *inl_ret = OneInputInstr::Create(&alloc, Opcode::RET, U64, {/* acc */}, {v, 0, addi});
    BasicBlock bb3 = BasicBlock::MakeBasicBlock({phi, addi, inl_ret});

    BasicBlock bb_end = BasicBlock::MakeBasicBlock({});

    // Blocks CFG
    BasicBlock::AddEdge(&bb_start, &bb0);

    BasicBlock::AddEdge(&bb0, &bb1);
    BasicBlock::AddEdge(&bb0, &bb2);

    BasicBlock::AddEdge(&bb1, &bb3);

    BasicBlock::AddEdge(&bb2, &bb3);

    BasicBlock::AddEdge(&bb3, &bb_end);

    Graph inlined_graph{&alloc, &bb_start, &bb_end, 2};
    inlined_graph.SetGraphForBasicBlocks({&bb_start, &bb0, &bb1, &bb2, &bb3, &bb_end});

    // Set inputs for instructions
    call->SetInputs(&alloc, InstrArg{0, &inlined_graph}, InstrArg{v, 0, movi1}, InstrArg{v, 1, movi2});
    phi->SetInputs(&alloc, InstrArg{v, 0, sub_versa}, InstrArg{v, 0, sub});

    // Set users for instructions
    const1->AddUsers({movi1});
    const5->AddUsers({movi2});
    movi1->AddUsers({call});
    movi2->AddUsers({call});
    call->AddUsers({sta});
    sta->AddUsers({ret});

    param0->AddUsers({cmp, sub, sub_versa});
    param1->AddUsers({cmp, sub, sub_versa});
    inl_const5->AddUsers({addi});
    cmp->AddUsers({jb});
    sub->AddUsers({phi});
    sub_versa->AddUsers({phi});
    phi->AddUsers({addi});
    addi->AddUsers({inl_ret});

    // Testing

    passes::Inlining inlPass{&curr_graph, &alloc};
    ASSERT_TRUE(inlPass.Run());

    // Check control flow
    ASSERT_TRUE(curr_bb_start.GetPreds().empty());
    ASSERT_EQ(curr_bb_start.GetSuccs().size(), 1);
    ASSERT_EQ(curr_bb_start.GetSuccs().front(), &curr_bb0);
    ASSERT_EQ(curr_bb0.GetPreds().size(), 1);
    ASSERT_EQ(curr_bb0.GetPreds().front(), &curr_bb_start);
    ASSERT_EQ(curr_bb0.GetSuccs().size(), 1);
    ASSERT_EQ(curr_bb0.GetSuccs().front(), &bb0);
    ASSERT_EQ(bb0.GetPreds().size(), 1);
    ASSERT_EQ(bb0.GetPreds().front(), &curr_bb0);
    ASSERT_EQ(bb0.GetSuccs().size(), 2);
    ASSERT_EQ(bb0.GetSuccs().at(0), &bb1);
    ASSERT_EQ(bb0.GetSuccs().at(1), &bb2);
    ASSERT_EQ(bb1.GetPreds().size(), 1);
    ASSERT_EQ(bb1.GetPreds().front(), &bb0);
    ASSERT_EQ(bb1.GetSuccs().size(), 1);
    ASSERT_EQ(bb1.GetSuccs().front(), &bb3);
    ASSERT_EQ(bb2.GetPreds().size(), 1);
    ASSERT_EQ(bb2.GetPreds().front(), &bb0);
    ASSERT_EQ(bb2.GetSuccs().size(), 1);
    ASSERT_EQ(bb2.GetSuccs().front(), &bb3);
    ASSERT_EQ(bb3.GetPreds().size(), 2);
    ASSERT_EQ(bb3.GetPreds().at(0), &bb1);
    ASSERT_EQ(bb3.GetPreds().at(1), &bb2);
    ASSERT_EQ(bb3.GetSuccs().size(), 1);
    auto *splitted_bb = bb3.GetSuccs().front();
    ASSERT_EQ(splitted_bb->GetFirstInstr()->GetOpcode(), Opcode::PHI);
    ASSERT_EQ(splitted_bb->GetLastInstr()->GetOpcode(), Opcode::RET);
    ASSERT_EQ(curr_bb_end.GetPreds().size(), 1);
    ASSERT_EQ(curr_bb_end.GetPreds().front(), splitted_bb);
    ASSERT_TRUE(curr_bb_end.GetSuccs().empty());

    // Check instructions
    auto start_bb_instrs = curr_bb_start.GetAllInstrs();
    ASSERT_EQ(start_bb_instrs.at(0), const1);
    ASSERT_EQ(start_bb_instrs.at(0)->GetPrev(), nullptr);
    ASSERT_EQ(start_bb_instrs.at(0)->GetNext(), const5);
    ASSERT_EQ(start_bb_instrs.at(1), const5);
    ASSERT_EQ(start_bb_instrs.at(1)->GetPrev(), const1);
    ASSERT_EQ(start_bb_instrs.at(1)->GetNext(), nullptr);
    auto curr_bb0_instrs = curr_bb0.GetAllInstrs();
    ASSERT_EQ(curr_bb0_instrs.at(0), movi1);
    ASSERT_EQ(curr_bb0_instrs.at(0)->GetPrev(), nullptr);
    ASSERT_EQ(curr_bb0_instrs.at(0)->GetNext(), movi2);
    ASSERT_EQ(curr_bb0_instrs.at(1), movi2);
    ASSERT_EQ(curr_bb0_instrs.at(1)->GetPrev(), movi1);
    ASSERT_EQ(curr_bb0_instrs.at(1)->GetNext(), nullptr);
    auto splitted_bb_instrs = splitted_bb->GetAllInstrs();
    ASSERT_EQ(splitted_bb_instrs.at(0)->GetPrev(), nullptr);
    ASSERT_EQ(splitted_bb_instrs.at(0)->GetNext(), sta);
    ASSERT_EQ(splitted_bb_instrs.at(1), sta);
    ASSERT_EQ(splitted_bb_instrs.at(1)->GetPrev()->GetOpcode(), Opcode::PHI);
    ASSERT_EQ(splitted_bb_instrs.at(1)->GetNext(), ret);
    ASSERT_EQ(splitted_bb_instrs.at(2), ret);
    ASSERT_EQ(splitted_bb_instrs.at(2)->GetPrev(), sta);
    ASSERT_EQ(splitted_bb_instrs.at(2)->GetNext(), nullptr);
}

TEST(inlining_tests, Example2) {
    /*
     * Constants and parameters are in bb_start(root) block

     * Current graph:
    u64 main():
        movi.u64 v0, 1           ┌
        movi.u64 v1, 5           |
        call.u64 foo, v0, v1     |  bb0
        sta.u64 v3               |
        ret.u64 v3               └

     * Inlined graph:
    u64 foo(u64 a0, u64 a1):
        cmp.u64 a0, a1          ┌  bb0
        jb sub_1_0              └
        sub.u64 v0, a0, a1      ┌  bb1
        jmp done                └
    sub_1_0:                    ┌  bb2
        sub.u64 v0, a1, a0      └
    done:                       ┌
        addi.u64 v0, v0, 5      |  bb3
        ret.void                └
     */

    constexpr auto a = InstrArg::Type::a;
    constexpr auto v = InstrArg::Type::v;
    constexpr auto imm = InstrArg::Type::imm;
    constexpr auto id = InstrArg::Type::id;
    constexpr auto U64 = InstrType::U64;

    Allocator alloc;

    // Creating current Graph
    ZeroInputInstr *const1 = ZeroInputInstr::Create(&alloc, Opcode::CONSTANT, U64, {imm, 1});
    ZeroInputInstr *const5 = ZeroInputInstr::Create(&alloc, Opcode::CONSTANT, U64, {imm, 5});
    BasicBlock curr_bb_start = BasicBlock::MakeBasicBlock({const1, const5});

    OneInputInstr *movi1 = OneInputInstr::Create(&alloc, Opcode::MOVI, U64, {v, 0}, {imm, 1, const1});
    OneInputInstr *movi2 = OneInputInstr::Create(&alloc, Opcode::MOVI, U64, {v, 1}, {imm, 5, const5});
    DynamicInputInstr *call = DynamicInputInstr::Create(&alloc, Opcode::CALL, U64, {/* acc */});
    OneInputInstr *sta = OneInputInstr::Create(&alloc, Opcode::STA, U64, {v, 3}, {/* acc */});
    OneInputInstr *ret = OneInputInstr::Create(&alloc, Opcode::RET, U64, {/* acc */}, {v, 3, sta});
    BasicBlock curr_bb0 = BasicBlock::MakeBasicBlock({movi1, movi2, call, sta, ret});

    BasicBlock curr_bb_end = BasicBlock::MakeBasicBlock({});

    // Blocks CFG
    BasicBlock::AddEdge(&curr_bb_start, &curr_bb0);
    BasicBlock::AddEdge(&curr_bb0, &curr_bb_end);

    Graph curr_graph{&alloc, &curr_bb_start, &curr_bb_end, 0};
    curr_graph.SetGraphForBasicBlocks({&curr_bb_start, &curr_bb0, &curr_bb_end});


    // Creating inlined Graph
    ZeroInputInstr *param0 = ZeroInputInstr::Create(&alloc, Opcode::PARAMETER, U64, {a, 0});
    ZeroInputInstr *param1 = ZeroInputInstr::Create(&alloc, Opcode::PARAMETER, U64, {a, 1});
    ZeroInputInstr *inl_const5 = ZeroInputInstr::Create(&alloc, Opcode::CONSTANT, U64, {imm, 5});
    BasicBlock bb_start = BasicBlock::MakeBasicBlock({param0, param1, inl_const5});

    TwoInputInstr *sub_versa = TwoInputInstr::Create(&alloc, Opcode::SUB, U64, {v, 0}, {a, 1, param1}, {a, 0, param0});
    BasicBlock bb2 = BasicBlock::MakeBasicBlock({sub_versa});

    TwoInputInstr *cmp = TwoInputInstr::Create(&alloc, Opcode::CMP, U64, {/* acc */}, {a, 0, param0}, {a, 1, param1});
    OneInputInstr *jb = OneInputInstr::Create(&alloc, Opcode::JB, U64, {id, 0, sub_versa}, {/* acc */});
    BasicBlock bb0 = BasicBlock::MakeBasicBlock({cmp, jb});

    DynamicInputInstr *phi = DynamicInputInstr::Create(&alloc, Opcode::PHI, U64, InstrArg{v, 0});

    TwoInputInstr *sub = TwoInputInstr::Create(&alloc, Opcode::SUB, U64, {v, 0}, {a, 0, param0}, {a, 1, param1});
    ZeroInputInstr *jmp = ZeroInputInstr::Create(&alloc, Opcode::JMP, U64, {id, 0, phi});  // target is "done" label
    BasicBlock bb1 = BasicBlock::MakeBasicBlock({sub, jmp});

    TwoInputInstr *addi = TwoInputInstr::Create(&alloc, Opcode::ADDI, U64, {v, 0}, {v, 0, phi}, {imm, 5, inl_const5});
    ZeroInputInstr *inl_ret = ZeroInputInstr::Create(&alloc, Opcode::RET_VOID, U64, {/* acc */});
    BasicBlock bb3 = BasicBlock::MakeBasicBlock({phi, addi, inl_ret});

    BasicBlock bb_end = BasicBlock::MakeBasicBlock({});

    // Blocks CFG
    BasicBlock::AddEdge(&bb_start, &bb0);

    BasicBlock::AddEdge(&bb0, &bb1);
    BasicBlock::AddEdge(&bb0, &bb2);

    BasicBlock::AddEdge(&bb1, &bb3);

    BasicBlock::AddEdge(&bb2, &bb3);

    BasicBlock::AddEdge(&bb3, &bb_end);

    Graph inlined_graph{&alloc, &bb_start, &bb_end, 2};
    inlined_graph.SetGraphForBasicBlocks({&bb_start, &bb0, &bb1, &bb2, &bb3, &bb_end});

    // Set inputs for instructions
    call->SetInputs(&alloc, InstrArg{0, &inlined_graph}, InstrArg{v, 0, movi1}, InstrArg{v, 1, movi2});
    phi->SetInputs(&alloc, InstrArg{v, 0, sub_versa}, InstrArg{v, 0, sub});

    // Set users for instructions
    const1->AddUsers({movi1});
    const5->AddUsers({movi2});
    movi1->AddUsers({call});
    movi2->AddUsers({call});
    call->AddUsers({sta});
    sta->AddUsers({ret});

    param0->AddUsers({cmp, sub, sub_versa});
    param1->AddUsers({cmp, sub, sub_versa});
    inl_const5->AddUsers({addi});
    cmp->AddUsers({jb});
    sub->AddUsers({phi});
    sub_versa->AddUsers({phi});
    phi->AddUsers({addi});
    addi->AddUsers({inl_ret});

    // Testing

    passes::Inlining inlPass{&curr_graph, &alloc};
    ASSERT_TRUE(inlPass.Run());

    // Check control flow
    ASSERT_TRUE(curr_bb_start.GetPreds().empty());
    ASSERT_EQ(curr_bb_start.GetSuccs().size(), 1);
    ASSERT_EQ(curr_bb_start.GetSuccs().front(), &curr_bb0);
    ASSERT_EQ(curr_bb0.GetPreds().size(), 1);
    ASSERT_EQ(curr_bb0.GetPreds().front(), &curr_bb_start);
    ASSERT_EQ(curr_bb0.GetSuccs().size(), 1);
    ASSERT_EQ(curr_bb0.GetSuccs().front(), &bb0);
    ASSERT_EQ(bb0.GetPreds().size(), 1);
    ASSERT_EQ(bb0.GetPreds().front(), &curr_bb0);
    ASSERT_EQ(bb0.GetSuccs().size(), 2);
    ASSERT_EQ(bb0.GetSuccs().at(0), &bb1);
    ASSERT_EQ(bb0.GetSuccs().at(1), &bb2);
    ASSERT_EQ(bb1.GetPreds().size(), 1);
    ASSERT_EQ(bb1.GetPreds().front(), &bb0);
    ASSERT_EQ(bb1.GetSuccs().size(), 1);
    ASSERT_EQ(bb1.GetSuccs().front(), &bb3);
    ASSERT_EQ(bb2.GetPreds().size(), 1);
    ASSERT_EQ(bb2.GetPreds().front(), &bb0);
    ASSERT_EQ(bb2.GetSuccs().size(), 1);
    ASSERT_EQ(bb2.GetSuccs().front(), &bb3);
    ASSERT_EQ(bb3.GetPreds().size(), 2);
    ASSERT_EQ(bb3.GetPreds().at(0), &bb1);
    ASSERT_EQ(bb3.GetPreds().at(1), &bb2);
    ASSERT_EQ(bb3.GetSuccs().size(), 1);
    auto *splitted_bb = bb3.GetSuccs().front();
    ASSERT_EQ(splitted_bb->GetFirstInstr()->GetOpcode(), Opcode::STA);
    ASSERT_EQ(splitted_bb->GetLastInstr()->GetOpcode(), Opcode::RET);
    ASSERT_EQ(curr_bb_end.GetPreds().size(), 1);
    ASSERT_EQ(curr_bb_end.GetPreds().front(), splitted_bb);
    ASSERT_TRUE(curr_bb_end.GetSuccs().empty());

    // Check instructions
    auto start_bb_instrs = curr_bb_start.GetAllInstrs();
    ASSERT_EQ(start_bb_instrs.at(0), const1);
    ASSERT_EQ(start_bb_instrs.at(0)->GetPrev(), nullptr);
    ASSERT_EQ(start_bb_instrs.at(0)->GetNext(), const5);
    ASSERT_EQ(start_bb_instrs.at(1), const5);
    ASSERT_EQ(start_bb_instrs.at(1)->GetPrev(), const1);
    ASSERT_EQ(start_bb_instrs.at(1)->GetNext(), nullptr);
    auto curr_bb0_instrs = curr_bb0.GetAllInstrs();
    ASSERT_EQ(curr_bb0_instrs.at(0), movi1);
    ASSERT_EQ(curr_bb0_instrs.at(0)->GetPrev(), nullptr);
    ASSERT_EQ(curr_bb0_instrs.at(0)->GetNext(), movi2);
    ASSERT_EQ(curr_bb0_instrs.at(1), movi2);
    ASSERT_EQ(curr_bb0_instrs.at(1)->GetPrev(), movi1);
    ASSERT_EQ(curr_bb0_instrs.at(1)->GetNext(), nullptr);
    auto splitted_bb_instrs = splitted_bb->GetAllInstrs();
    ASSERT_EQ(splitted_bb_instrs.at(0), sta);
    ASSERT_EQ(splitted_bb_instrs.at(0)->GetPrev(), nullptr);
    ASSERT_EQ(splitted_bb_instrs.at(0)->GetNext(), ret);
    ASSERT_EQ(splitted_bb_instrs.at(1), ret);
    ASSERT_EQ(splitted_bb_instrs.at(1)->GetPrev(), sta);
    ASSERT_EQ(splitted_bb_instrs.at(1)->GetNext(), nullptr);
}

TEST(inlining_tests, Example3) {
    /*
     * Constants and parameters are in bb_start(root) block

     * Current graph:
    u64 main():
        movi.u64 v0, 1           ┌
        movi.u64 v1, 5           |
        call.u64 foo, v0, v1     |  bb0
        sta.u64 v3               |
        ret.u64 v3               └

     * Inlined graph:
    u64 foo(u64 a0, u64 a1):
        cmp.u64 a0, a1          ┌  bb0
        jb sub_1_0              └
        sub.u64 v0, a0, a1      ┌  bb1
        ret.u64 v0              └
    sub_1_0:                    ┌  bb2
        sub.u64 v0, a1, a0      |
        ret.u64 v0              └
     */

    constexpr auto a = InstrArg::Type::a;
    constexpr auto v = InstrArg::Type::v;
    constexpr auto imm = InstrArg::Type::imm;
    constexpr auto id = InstrArg::Type::id;
    constexpr auto U64 = InstrType::U64;

    Allocator alloc;

    // Creating current Graph
    ZeroInputInstr *const1 = ZeroInputInstr::Create(&alloc, Opcode::CONSTANT, U64, {imm, 1});
    ZeroInputInstr *const5 = ZeroInputInstr::Create(&alloc, Opcode::CONSTANT, U64, {imm, 5});
    BasicBlock curr_bb_start = BasicBlock::MakeBasicBlock({const1, const5});

    OneInputInstr *movi1 = OneInputInstr::Create(&alloc, Opcode::MOVI, U64, {v, 0}, {imm, 1, const1});
    OneInputInstr *movi2 = OneInputInstr::Create(&alloc, Opcode::MOVI, U64, {v, 1}, {imm, 5, const5});
    DynamicInputInstr *call = DynamicInputInstr::Create(&alloc, Opcode::CALL, U64, {/* acc */});
    OneInputInstr *sta = OneInputInstr::Create(&alloc, Opcode::STA, U64, {v, 3}, {/* acc */});
    OneInputInstr *ret = OneInputInstr::Create(&alloc, Opcode::RET, U64, {/* acc */}, {v, 3, sta});
    BasicBlock curr_bb0 = BasicBlock::MakeBasicBlock({movi1, movi2, call, sta, ret});

    BasicBlock curr_bb_end = BasicBlock::MakeBasicBlock({});

    // Blocks CFG
    BasicBlock::AddEdge(&curr_bb_start, &curr_bb0);
    BasicBlock::AddEdge(&curr_bb0, &curr_bb_end);

    Graph curr_graph{&alloc, &curr_bb_start, &curr_bb_end, 0};
    curr_graph.SetGraphForBasicBlocks({&curr_bb_start, &curr_bb0, &curr_bb_end});


    // Creating inlined Graph
    ZeroInputInstr *param0 = ZeroInputInstr::Create(&alloc, Opcode::PARAMETER, U64, {a, 0});
    ZeroInputInstr *param1 = ZeroInputInstr::Create(&alloc, Opcode::PARAMETER, U64, {a, 1});
    ZeroInputInstr *inl_const5 = ZeroInputInstr::Create(&alloc, Opcode::CONSTANT, U64, {imm, 5});
    BasicBlock bb_start = BasicBlock::MakeBasicBlock({param0, param1, inl_const5});

    TwoInputInstr *sub_versa = TwoInputInstr::Create(&alloc, Opcode::SUB, U64, {v, 0}, {a, 1, param1}, {a, 0, param0});
    OneInputInstr *inl_ret2 = OneInputInstr::Create(&alloc, Opcode::RET, U64, {/* acc */}, {v, 0, sub_versa});
    BasicBlock bb2 = BasicBlock::MakeBasicBlock({sub_versa, inl_ret2});

    TwoInputInstr *cmp = TwoInputInstr::Create(&alloc, Opcode::CMP, U64, {/* acc */}, {a, 0, param0}, {a, 1, param1});
    OneInputInstr *jb = OneInputInstr::Create(&alloc, Opcode::JB, U64, {id, 0, sub_versa}, {/* acc */});
    BasicBlock bb0 = BasicBlock::MakeBasicBlock({cmp, jb});

    TwoInputInstr *sub = TwoInputInstr::Create(&alloc, Opcode::SUB, U64, {v, 0}, {a, 0, param0}, {a, 1, param1});
    OneInputInstr *inl_ret1 = OneInputInstr::Create(&alloc, Opcode::RET, U64, {/* acc */}, {v, 0, sub});
    BasicBlock bb1 = BasicBlock::MakeBasicBlock({sub, inl_ret1});

    BasicBlock bb_end = BasicBlock::MakeBasicBlock({});

    // Blocks CFG
    BasicBlock::AddEdge(&bb_start, &bb0);

    BasicBlock::AddEdge(&bb0, &bb1);
    BasicBlock::AddEdge(&bb0, &bb2);

    BasicBlock::AddEdge(&bb1, &bb_end);

    BasicBlock::AddEdge(&bb2, &bb_end);

    Graph inlined_graph{&alloc, &bb_start, &bb_end, 2};
    inlined_graph.SetGraphForBasicBlocks({&bb_start, &bb0, &bb1, &bb2, &bb_end});

    // Set inputs for instructions
    call->SetInputs(&alloc, InstrArg{0, &inlined_graph}, InstrArg{v, 0, movi1}, InstrArg{v, 1, movi2});

    // Set users for instructions
    const1->AddUsers({movi1});
    const5->AddUsers({movi2});
    movi1->AddUsers({call});
    movi2->AddUsers({call});
    call->AddUsers({sta});
    sta->AddUsers({ret});

    param0->AddUsers({cmp, sub, sub_versa});
    param1->AddUsers({cmp, sub, sub_versa});
    cmp->AddUsers({jb});
    sub->AddUsers({inl_ret1});
    sub_versa->AddUsers({inl_ret2});

    // Testing

    passes::Inlining inlPass{&curr_graph, &alloc};
    ASSERT_TRUE(inlPass.Run());

    // Check control flow
    ASSERT_TRUE(curr_bb_start.GetPreds().empty());
    ASSERT_EQ(curr_bb_start.GetSuccs().size(), 1);
    ASSERT_EQ(curr_bb_start.GetSuccs().front(), &curr_bb0);
    ASSERT_EQ(curr_bb0.GetPreds().size(), 1);
    ASSERT_EQ(curr_bb0.GetPreds().front(), &curr_bb_start);
    ASSERT_EQ(curr_bb0.GetSuccs().size(), 1);
    ASSERT_EQ(curr_bb0.GetSuccs().front(), &bb0);
    ASSERT_EQ(bb0.GetPreds().size(), 1);
    ASSERT_EQ(bb0.GetPreds().front(), &curr_bb0);
    ASSERT_EQ(bb0.GetSuccs().size(), 2);
    ASSERT_EQ(bb0.GetSuccs().at(0), &bb1);
    ASSERT_EQ(bb0.GetSuccs().at(1), &bb2);
    ASSERT_EQ(bb1.GetPreds().size(), 1);
    ASSERT_EQ(bb1.GetPreds().front(), &bb0);
    ASSERT_EQ(bb1.GetSuccs().size(), 1);
    auto *splitted_bb = bb1.GetSuccs().front();
    ASSERT_EQ(bb2.GetPreds().size(), 1);
    ASSERT_EQ(bb2.GetPreds().front(), &bb0);
    ASSERT_EQ(bb2.GetSuccs().size(), 1);
    ASSERT_EQ(bb2.GetSuccs().front(), splitted_bb);
    ASSERT_EQ(splitted_bb->GetFirstInstr()->GetOpcode(), Opcode::PHI);
    ASSERT_EQ(splitted_bb->GetLastInstr()->GetOpcode(), Opcode::RET);
    ASSERT_EQ(curr_bb_end.GetPreds().size(), 1);
    ASSERT_EQ(curr_bb_end.GetPreds().front(), splitted_bb);
    ASSERT_TRUE(curr_bb_end.GetSuccs().empty());

    // Check instructions
    auto start_bb_instrs = curr_bb_start.GetAllInstrs();
    ASSERT_EQ(start_bb_instrs.at(0), const1);
    ASSERT_EQ(start_bb_instrs.at(0)->GetPrev(), nullptr);
    ASSERT_EQ(start_bb_instrs.at(0)->GetNext(), const5);
    ASSERT_EQ(start_bb_instrs.at(1), const5);
    ASSERT_EQ(start_bb_instrs.at(1)->GetPrev(), const1);
    ASSERT_EQ(start_bb_instrs.at(1)->GetNext(), nullptr);
    auto curr_bb0_instrs = curr_bb0.GetAllInstrs();
    ASSERT_EQ(curr_bb0_instrs.at(0), movi1);
    ASSERT_EQ(curr_bb0_instrs.at(0)->GetPrev(), nullptr);
    ASSERT_EQ(curr_bb0_instrs.at(0)->GetNext(), movi2);
    ASSERT_EQ(curr_bb0_instrs.at(1), movi2);
    ASSERT_EQ(curr_bb0_instrs.at(1)->GetPrev(), movi1);
    ASSERT_EQ(curr_bb0_instrs.at(1)->GetNext(), nullptr);
    auto splitted_bb_instrs = splitted_bb->GetAllInstrs();
    ASSERT_EQ(splitted_bb_instrs.at(0)->GetPrev(), nullptr);
    ASSERT_EQ(splitted_bb_instrs.at(0)->GetNext(), sta);
    ASSERT_EQ(splitted_bb_instrs.at(1), sta);
    ASSERT_EQ(splitted_bb_instrs.at(1)->GetPrev()->GetOpcode(), Opcode::PHI);
    ASSERT_EQ(splitted_bb_instrs.at(1)->GetNext(), ret);
    ASSERT_EQ(splitted_bb_instrs.at(2), ret);
    ASSERT_EQ(splitted_bb_instrs.at(2)->GetPrev(), sta);
    ASSERT_EQ(splitted_bb_instrs.at(2)->GetNext(), nullptr);
}

TEST(inlining_tests, Example4) {
    /*
     * Constants and parameters are in bb_start(root) block

     * Current graph:
    u64 main():
        movi.u64 v0, 1           ┌
        movi.u64 v1, 5           |
        call.u64 foo, v0, v1     |  bb0
        sta.u64 v3               |
        ret.u64 v3               └

     * Inlined graph:
    u64 foo(u64 a0, u64 a1):
        cmp.u64 a0, a1          ┌  bb0
        jb sub_1_0              └
        sub.u64 v0, a0, a1      ┌  bb1
        ret.u64 v0              └
    sub_1_0:                    ┌  bb2
        throw                   └
     */

    constexpr auto a = InstrArg::Type::a;
    constexpr auto v = InstrArg::Type::v;
    constexpr auto imm = InstrArg::Type::imm;
    constexpr auto id = InstrArg::Type::id;
    constexpr auto U64 = InstrType::U64;

    Allocator alloc;

    // Creating current Graph
    ZeroInputInstr *const1 = ZeroInputInstr::Create(&alloc, Opcode::CONSTANT, U64, {imm, 1, nullptr});
    ZeroInputInstr *const5 = ZeroInputInstr::Create(&alloc, Opcode::CONSTANT, U64, {imm, 5});
    BasicBlock curr_bb_start = BasicBlock::MakeBasicBlock({const1, const5});

    OneInputInstr *movi1 = OneInputInstr::Create(&alloc, Opcode::MOVI, U64, {v, 0}, {imm, 1, const1});
    OneInputInstr *movi2 = OneInputInstr::Create(&alloc, Opcode::MOVI, U64, {v, 1}, {imm, 5, const5});
    DynamicInputInstr *call = DynamicInputInstr::Create(&alloc, Opcode::CALL, U64, {/* acc */});
    OneInputInstr *sta = OneInputInstr::Create(&alloc, Opcode::STA, U64, {v, 3}, {call});
    OneInputInstr *ret = OneInputInstr::Create(&alloc, Opcode::RET, U64, {/* acc */}, {v, 3, sta});
    BasicBlock curr_bb0 = BasicBlock::MakeBasicBlock({movi1, movi2, call, sta, ret});

    BasicBlock curr_bb_end = BasicBlock::MakeBasicBlock({});

    // Blocks CFG
    BasicBlock::AddEdge(&curr_bb_start, &curr_bb0);
    BasicBlock::AddEdge(&curr_bb0, &curr_bb_end);

    Graph curr_graph{&alloc, &curr_bb_start, &curr_bb_end, 0};
    curr_graph.SetGraphForBasicBlocks({&curr_bb_start, &curr_bb0, &curr_bb_end});


    // Creating inlined Graph
    ZeroInputInstr *param0 = ZeroInputInstr::Create(&alloc, Opcode::PARAMETER, U64, {a, 0});
    ZeroInputInstr *param1 = ZeroInputInstr::Create(&alloc, Opcode::PARAMETER, U64, {a, 1});
    ZeroInputInstr *inl_const5 = ZeroInputInstr::Create(&alloc, Opcode::CONSTANT, U64, {imm, 5});
    BasicBlock bb_start = BasicBlock::MakeBasicBlock({param0, param1, inl_const5});

    ZeroInputInstr *inl_throw = ZeroInputInstr::Create(&alloc, Opcode::THROW, U64, {/* acc */});
    BasicBlock bb2 = BasicBlock::MakeBasicBlock({inl_throw});

    TwoInputInstr *cmp = TwoInputInstr::Create(&alloc, Opcode::CMP, U64, {/* acc */}, {a, 0, param0}, {a, 1, param1});
    OneInputInstr *jb = OneInputInstr::Create(&alloc, Opcode::JB, U64, {id, 0, inl_throw}, {/* acc */});
    BasicBlock bb0 = BasicBlock::MakeBasicBlock({cmp, jb});

    TwoInputInstr *sub = TwoInputInstr::Create(&alloc, Opcode::SUB, U64, {v, 0}, {a, 0, param0}, {a, 1, param1});
    OneInputInstr *inl_ret1 = OneInputInstr::Create(&alloc, Opcode::RET, U64, {/* acc */}, {v, 0, sub});
    BasicBlock bb1 = BasicBlock::MakeBasicBlock({sub, inl_ret1});

    BasicBlock bb_end = BasicBlock::MakeBasicBlock({});

    // Blocks CFG
    BasicBlock::AddEdge(&bb_start, &bb0);

    BasicBlock::AddEdge(&bb0, &bb1);
    BasicBlock::AddEdge(&bb0, &bb2);

    BasicBlock::AddEdge(&bb1, &bb_end);

    BasicBlock::AddEdge(&bb2, &bb_end);

    Graph inlined_graph{&alloc, &bb_start, &bb_end, 2};
    inlined_graph.SetGraphForBasicBlocks({&bb_start, &bb0, &bb1, &bb2, &bb_end});

    // Set inputs for instructions
    call->SetInputs(&alloc, InstrArg{0, &inlined_graph}, InstrArg{v, 0, movi1}, InstrArg{v, 1, movi2});

    // Set users for instructions
    const1->AddUsers({movi1});
    const5->AddUsers({movi2});
    movi1->AddUsers({call});
    movi2->AddUsers({call});
    call->AddUsers({sta});
    sta->AddUsers({ret});

    param0->AddUsers({cmp, sub});
    param1->AddUsers({cmp, sub});
    cmp->AddUsers({jb});
    sub->AddUsers({inl_ret1});

    // Testing

    passes::Inlining inlPass{&curr_graph, &alloc};
    ASSERT_TRUE(inlPass.Run());

    // Check control flow
    ASSERT_TRUE(curr_bb_start.GetPreds().empty());
    ASSERT_EQ(curr_bb_start.GetSuccs().size(), 1);
    ASSERT_EQ(curr_bb_start.GetSuccs().front(), &curr_bb0);
    ASSERT_EQ(curr_bb0.GetPreds().size(), 1);
    ASSERT_EQ(curr_bb0.GetPreds().front(), &curr_bb_start);
    ASSERT_EQ(curr_bb0.GetSuccs().size(), 1);
    ASSERT_EQ(curr_bb0.GetSuccs().front(), &bb0);
    ASSERT_EQ(bb0.GetPreds().size(), 1);
    ASSERT_EQ(bb0.GetPreds().front(), &curr_bb0);
    ASSERT_EQ(bb0.GetSuccs().size(), 2);
    ASSERT_EQ(bb0.GetSuccs().at(0), &bb1);
    ASSERT_EQ(bb0.GetSuccs().at(1), &bb2);
    ASSERT_EQ(bb1.GetPreds().size(), 1);
    ASSERT_EQ(bb1.GetPreds().front(), &bb0);
    ASSERT_EQ(bb1.GetSuccs().size(), 1);
    auto *splitted_bb = bb1.GetSuccs().front();
    ASSERT_EQ(bb2.GetPreds().size(), 1);
    ASSERT_EQ(bb2.GetPreds().front(), &bb0);
    ASSERT_EQ(bb2.GetSuccs().size(), 1);
    ASSERT_EQ(bb2.GetSuccs().front(), splitted_bb);
    ASSERT_EQ(splitted_bb->GetFirstInstr()->GetOpcode(), Opcode::STA);
    ASSERT_EQ(splitted_bb->GetLastInstr()->GetOpcode(), Opcode::RET);
    ASSERT_EQ(curr_bb_end.GetPreds().size(), 1);
    ASSERT_EQ(curr_bb_end.GetPreds().front(), splitted_bb);
    ASSERT_TRUE(curr_bb_end.GetSuccs().empty());

    // Check instructions
    auto start_bb_instrs = curr_bb_start.GetAllInstrs();
    ASSERT_EQ(start_bb_instrs.at(0), const1);
    ASSERT_EQ(start_bb_instrs.at(0)->GetPrev(), nullptr);
    ASSERT_EQ(start_bb_instrs.at(0)->GetNext(), const5);
    ASSERT_EQ(start_bb_instrs.at(1), const5);
    ASSERT_EQ(start_bb_instrs.at(1)->GetPrev(), const1);
    ASSERT_EQ(start_bb_instrs.at(1)->GetNext(), nullptr);
    auto curr_bb0_instrs = curr_bb0.GetAllInstrs();
    ASSERT_EQ(curr_bb0_instrs.at(0), movi1);
    ASSERT_EQ(curr_bb0_instrs.at(0)->GetPrev(), nullptr);
    ASSERT_EQ(curr_bb0_instrs.at(0)->GetNext(), movi2);
    ASSERT_EQ(curr_bb0_instrs.at(1), movi2);
    ASSERT_EQ(curr_bb0_instrs.at(1)->GetPrev(), movi1);
    ASSERT_EQ(curr_bb0_instrs.at(1)->GetNext(), nullptr);
    auto splitted_bb_instrs = splitted_bb->GetAllInstrs();
    ASSERT_EQ(splitted_bb_instrs.at(0), sta);
    ASSERT_EQ(splitted_bb_instrs.at(0)->GetPrev(), nullptr);
    ASSERT_EQ(splitted_bb_instrs.at(0)->GetNext(), ret);
    ASSERT_EQ(splitted_bb_instrs.at(1), ret);
    ASSERT_EQ(splitted_bb_instrs.at(1)->GetPrev(), sta);
    ASSERT_EQ(splitted_bb_instrs.at(1)->GetNext(), nullptr);
}

}  // namespace compiler::test

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
