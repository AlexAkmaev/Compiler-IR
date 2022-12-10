#include <gtest/gtest.h>

#include "Graph.h"
#include "pass.h"

namespace compiler::test {

class GraphTest : public testing::Test {
public:
    GraphTest() {
        SetFirstGraph();
        SetSecondGraph();
        SetThirdGraph();
    }

    virtual void SetUp() {}

    virtual void TearDown() {}

    void SetFirstGraph() {
        /*
         *    `          A
         *               ↓
         *               B
         *             ↙   ↘
         *           C   E ← F
         *            ↘ ↙   ↙
         *             D ← G
         *
         */
        bb_holder1_.resize(7);
        BasicBlock  &A = bb_holder1_.at(0),
                    &B = bb_holder1_.at(1),
                    &C = bb_holder1_.at(2),
                    &D = bb_holder1_.at(3),
                    &E = bb_holder1_.at(4),
                    &F = bb_holder1_.at(5),
                    &G = bb_holder1_.at(6);

        BasicBlock::AddEdge(&A, &B);

        BasicBlock::AddEdge(&B, &C);
        BasicBlock::AddEdge(&B, &F);

        BasicBlock::AddEdge(&C, &D);

        BasicBlock::AddEdge(&E, &D);

        BasicBlock::AddEdge(&F, &E);
        BasicBlock::AddEdge(&F, &G);

        BasicBlock::AddEdge(&G, &D);

        graph1_.SetRoot(&A);
        graph1_.SetEnd(&D);
        graph1_.SetParamsNum(0);
        graph1_.SetGraphForBasicBlocks({&A, &B, &C, &D, &E, &F, &G});

        EXPECT_EQ(A.GetId(), 0);
        EXPECT_EQ(D.GetId(), 1);
        EXPECT_EQ(B.GetId(), 2);
        EXPECT_EQ(C.GetId(), 3);
        EXPECT_EQ(E.GetId(), 4);
        EXPECT_EQ(F.GetId(), 5);
        EXPECT_EQ(G.GetId(), 6);
    }

    void SetSecondGraph() {
        /*
         *    `          A
         *               ↓
         *           ┌-→ B
         *           |   ↓  ↘
         *           |   C ← J
         *           |  ↓ ↑
         *           |   D
         *           |   ↓
         *           |   E
         *           |  ↓ ↑
         *           |   F
         *           |   ↓
         *           H ← G → I
         *                 ↙
         *               K
         *
         */
        bb_holder2_.resize(11);
        BasicBlock  &A = bb_holder2_.at(0),
                    &B = bb_holder2_.at(1),
                    &C = bb_holder2_.at(2),
                    &D = bb_holder2_.at(3),
                    &E = bb_holder2_.at(4),
                    &F = bb_holder2_.at(5),
                    &G = bb_holder2_.at(6),
                    &H = bb_holder2_.at(7),
                    &I = bb_holder2_.at(8),
                    &J = bb_holder2_.at(9),
                    &K = bb_holder2_.at(10);

        BasicBlock::AddEdge(&A, &B);

        BasicBlock::AddEdge(&B, &C);
        BasicBlock::AddEdge(&B, &J);

        BasicBlock::AddEdge(&C, &D);

        BasicBlock::AddEdge(&D, &C);
        BasicBlock::AddEdge(&D, &E);

        BasicBlock::AddEdge(&E, &F);

        BasicBlock::AddEdge(&F, &E);
        BasicBlock::AddEdge(&F, &G);

        BasicBlock::AddEdge(&G, &I);
        BasicBlock::AddEdge(&G, &H);

        BasicBlock::AddEdge(&H, &B);

        BasicBlock::AddEdge(&I, &K);

        BasicBlock::AddEdge(&J, &C);

        graph2_.SetRoot(&A);
        graph2_.SetEnd(&K);
        graph2_.SetParamsNum(0);
        graph2_.SetGraphForBasicBlocks({&A, &B, &C, &D, &E, &F, &G, &H, &I, &J, &K});

        EXPECT_EQ(A.GetId(), 0);
        EXPECT_EQ(K.GetId(), 1);
        EXPECT_EQ(B.GetId(), 2);
        EXPECT_EQ(C.GetId(), 3);
        EXPECT_EQ(D.GetId(), 4);
        EXPECT_EQ(E.GetId(), 5);
        EXPECT_EQ(F.GetId(), 6);
        EXPECT_EQ(G.GetId(), 7);
        EXPECT_EQ(H.GetId(), 8);
        EXPECT_EQ(I.GetId(), 9);
        EXPECT_EQ(J.GetId(), 10);
    }

    void SetThirdGraph() {
        /*
         *    `          A
         *               ↓
         *        ┌----→ B
         *        |    ↙ ↓
         *        |  E   C ←┐
         *        |  ↓ ↘ ↓  |
         *        └- F   D  |
         *           ↓   ↓  |
         *           H → G -┘
         *             ↘ ↓
         *               I
         *
         */
        bb_holder3_.resize(9);
        BasicBlock  &A = bb_holder3_.at(0),
                    &B = bb_holder3_.at(1),
                    &C = bb_holder3_.at(2),
                    &D = bb_holder3_.at(3),
                    &E = bb_holder3_.at(4),
                    &F = bb_holder3_.at(5),
                    &G = bb_holder3_.at(6),
                    &H = bb_holder3_.at(7),
                    &I = bb_holder3_.at(8);

        BasicBlock::AddEdge(&A, &B);

        BasicBlock::AddEdge(&B, &E);
        BasicBlock::AddEdge(&B, &C);

        BasicBlock::AddEdge(&C, &D);

        BasicBlock::AddEdge(&D, &G);

        BasicBlock::AddEdge(&E, &F);
        BasicBlock::AddEdge(&E, &D);

        BasicBlock::AddEdge(&F, &B);
        BasicBlock::AddEdge(&F, &H);

        BasicBlock::AddEdge(&G, &I);
        BasicBlock::AddEdge(&G, &C);

        BasicBlock::AddEdge(&H, &I);
        BasicBlock::AddEdge(&H, &G);

        graph3_.SetRoot(&A);
        graph3_.SetEnd(&I);
        graph3_.SetParamsNum(0);
        graph3_.SetGraphForBasicBlocks({&A, &B, &C, &D, &E, &F, &G, &H, &I});

        EXPECT_EQ(A.GetId(), 0);
        EXPECT_EQ(I.GetId(), 1);
        EXPECT_EQ(B.GetId(), 2);
        EXPECT_EQ(C.GetId(), 3);
        EXPECT_EQ(D.GetId(), 4);
        EXPECT_EQ(E.GetId(), 5);
        EXPECT_EQ(F.GetId(), 6);
        EXPECT_EQ(G.GetId(), 7);
        EXPECT_EQ(H.GetId(), 8);
    }

    Graph GetFirstGraph() const {
        return graph1_;
    }

    Graph GetSecondGraph() const {
        return graph2_;
    }

    Graph GetThirdGraph() const {
        return graph3_;
    }

private:
    Graph graph1_;
    Graph graph2_;
    Graph graph3_;

    std::vector<BasicBlock> bb_holder1_;
    std::vector<BasicBlock> bb_holder2_;
    std::vector<BasicBlock> bb_holder3_;
};

TEST_F(GraphTest, simple_rpo) {
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

TEST_F(GraphTest, example1_rpo) {
    Graph graph = GetFirstGraph();
    passes::Traversal tr{&graph};
    tr.Run();
    BlocksVector rpo = tr.getRPO();

    ASSERT_EQ(rpo.size(), 7);
    ASSERT_EQ(rpo.at(0)->GetId(), 0);   // A
    ASSERT_EQ(rpo.at(1)->GetId(), 2);   // B
    ASSERT_EQ(rpo.at(2)->GetId(), 5);   // F
    ASSERT_EQ(rpo.at(3)->GetId(), 6);   // G
    ASSERT_EQ(rpo.at(4)->GetId(), 4);   // E
    ASSERT_EQ(rpo.at(5)->GetId(), 3);   // C
    ASSERT_EQ(rpo.at(6)->GetId(), 1);   // D
}

TEST_F(GraphTest, example2_rpo) {
    Graph graph = GetSecondGraph();
    passes::Traversal tr{&graph};
    tr.Run();
    BlocksVector rpo = tr.getRPO();

    ASSERT_EQ(rpo.size(), 11);
    ASSERT_EQ(rpo.at(0)->GetId(), 0);   // A
    ASSERT_EQ(rpo.at(1)->GetId(), 2);   // B
    ASSERT_EQ(rpo.at(2)->GetId(), 10);  // J
    ASSERT_EQ(rpo.at(3)->GetId(), 3);   // C
    ASSERT_EQ(rpo.at(4)->GetId(), 4);   // D
    ASSERT_EQ(rpo.at(5)->GetId(), 5);   // E
    ASSERT_EQ(rpo.at(6)->GetId(), 6);   // F
    ASSERT_EQ(rpo.at(7)->GetId(), 7);   // G
    ASSERT_EQ(rpo.at(8)->GetId(), 8);   // H
    ASSERT_EQ(rpo.at(9)->GetId(), 9);   // I
    ASSERT_EQ(rpo.at(10)->GetId(), 1);  // K
}

TEST_F(GraphTest, example3_rpo) {
    Graph graph = GetThirdGraph();
    passes::Traversal tr{&graph};
    tr.Run();
    BlocksVector rpo = tr.getRPO();

    ASSERT_EQ(rpo.size(), 9);
    ASSERT_EQ(rpo.at(0)->GetId(), 0);   // A
    ASSERT_EQ(rpo.at(1)->GetId(), 2);   // B
    ASSERT_EQ(rpo.at(2)->GetId(), 5);   // E
    ASSERT_EQ(rpo.at(3)->GetId(), 6);   // F
    ASSERT_EQ(rpo.at(4)->GetId(), 8);   // H
    ASSERT_EQ(rpo.at(5)->GetId(), 7);   // G
    ASSERT_EQ(rpo.at(6)->GetId(), 3);   // C
    ASSERT_EQ(rpo.at(7)->GetId(), 4);   // D
    ASSERT_EQ(rpo.at(8)->GetId(), 1);   // I
}

}  // namespace compiler::test

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}