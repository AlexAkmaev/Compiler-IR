#include <gtest/gtest.h>

#include "Graph.h"
#include "pass.h"

namespace compiler::test {

TEST(graph_tests, simple_rpo) {
    /*
     *               A
     *             ↙   ↘
     *           B       C
     *             ↘   ↙
     *               D
     *
     */
    BasicBlock A, B, C, D;

    BasicBlock::AddEdge(&A, &B);
    BasicBlock::AddEdge(&A, &C);

    BasicBlock::AddEdge(&B, &D);

    BasicBlock::AddEdge(&C, &D);

    Graph graph{&A, &D, 0};
    graph.SetGraphForBasicBlocks({&A, &B, &C, &D});
    ASSERT_EQ(A.GetId(), 0);
    ASSERT_EQ(B.GetId(), 2);
    ASSERT_EQ(C.GetId(), 3);
    ASSERT_EQ(D.GetId(), 1);

    passes::Traversal tr{&graph};
    tr.Run();
    BlocksVector rpo = tr.getRPO();
    ASSERT_EQ(rpo.size(), 4);
    ASSERT_EQ(rpo.at(0)->GetId(), 0);
    ASSERT_EQ(rpo.at(1)->GetId(), 3);
    ASSERT_EQ(rpo.at(2)->GetId(), 2);
    ASSERT_EQ(rpo.at(3)->GetId(), 1);
}

TEST(graph_tests, example1_rpo) {
    /*
     *               A
     *               ↓
     *               B
     *             ↙   ↘
     *           C   E ← F
     *            ↘ ↙   ↙
     *             D ← G
     *           ↙
     *         End
     *
     */
    BasicBlock A, B, C, D, E, F, G, End;

    BasicBlock::AddEdge(&A, &B);

    BasicBlock::AddEdge(&B, &C);
    BasicBlock::AddEdge(&B, &F);

    BasicBlock::AddEdge(&C, &D);

    BasicBlock::AddEdge(&E, &D);

    BasicBlock::AddEdge(&F, &E);
    BasicBlock::AddEdge(&F, &G);

    BasicBlock::AddEdge(&G, &D);

    BasicBlock::AddEdge(&G, &End);

    Graph graph{&A, &End, 0};
    graph.SetGraphForBasicBlocks({&A, &B, &C, &D, &E, &F, &G, &End});
    ASSERT_EQ(A.GetId(), 0);
    ASSERT_EQ(End.GetId(), 1);
    ASSERT_EQ(B.GetId(), 2);
    ASSERT_EQ(C.GetId(), 3);
    ASSERT_EQ(D.GetId(), 4);
    ASSERT_EQ(E.GetId(), 5);
    ASSERT_EQ(F.GetId(), 6);
    ASSERT_EQ(G.GetId(), 7);

    passes::Traversal tr{&graph};
    tr.Run();
    BlocksVector rpo = tr.getRPO();
    ASSERT_EQ(rpo.size(), 8);
    ASSERT_EQ(rpo.at(0)->GetId(), 0);
    ASSERT_EQ(rpo.at(1)->GetId(), 2);
    ASSERT_EQ(rpo.at(2)->GetId(), 6);
    ASSERT_EQ(rpo.at(3)->GetId(), 7);
    ASSERT_EQ(rpo.at(4)->GetId(), 1);
    ASSERT_EQ(rpo.at(5)->GetId(), 5);
    ASSERT_EQ(rpo.at(6)->GetId(), 3);
    ASSERT_EQ(rpo.at(7)->GetId(), 4);
}

}  // namespace compiler::test

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}