#include <gtest/gtest.h>

#include "graph.h"
#include "pass.h"
#include "loop_analyzer.h"

namespace compiler::test {

// Graph 1 BasicBlocks enumeration
namespace G1_BB {
uint8_t A = 0, D = 1, B = 2, C = 3, E = 4, F = 5, G = 6;
}

// Graph 2 BasicBlocks enumeration
namespace G2_BB {
uint8_t A = 0, K = 1, B = 2, C = 3, D = 4, E = 5, F = 6, G = 7, H = 8, I = 9, J = 10;
}

// Graph 3 BasicBlocks enumeration
namespace G3_BB {
uint8_t A = 0, I = 1, B = 2, C = 3, D = 4, E = 5, F = 6, G = 7, H = 8;
}

class GraphTest : public testing::Test {
public:
    GraphTest() : graph1_(&allocator_), graph2_(&allocator_), graph3_(&allocator_) {
        SetFirstGraph();
        SetSecondGraph();
        SetThirdGraph();
    }

    virtual void SetUp() {}

    virtual void TearDown() {}
    
    Allocator *GetAllocator() {
        return &allocator_;
    }

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

        EXPECT_EQ(A.GetId(), G1_BB::A);
        EXPECT_EQ(D.GetId(), G1_BB::D);
        EXPECT_EQ(B.GetId(), G1_BB::B);
        EXPECT_EQ(C.GetId(), G1_BB::C);
        EXPECT_EQ(E.GetId(), G1_BB::E);
        EXPECT_EQ(F.GetId(), G1_BB::F);
        EXPECT_EQ(G.GetId(), G1_BB::G);
    }

    void SetSecondGraph() {
        /*
         *               A
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

        EXPECT_EQ(A.GetId(), G2_BB::A);
        EXPECT_EQ(K.GetId(), G2_BB::K);
        EXPECT_EQ(B.GetId(), G2_BB::B);
        EXPECT_EQ(C.GetId(), G2_BB::C);
        EXPECT_EQ(D.GetId(), G2_BB::D);
        EXPECT_EQ(E.GetId(), G2_BB::E);
        EXPECT_EQ(F.GetId(), G2_BB::F);
        EXPECT_EQ(G.GetId(), G2_BB::G);
        EXPECT_EQ(H.GetId(), G2_BB::H);
        EXPECT_EQ(I.GetId(), G2_BB::I);
        EXPECT_EQ(J.GetId(), G2_BB::J);
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

        EXPECT_EQ(A.GetId(), G3_BB::A);
        EXPECT_EQ(I.GetId(), G3_BB::I);
        EXPECT_EQ(B.GetId(), G3_BB::B);
        EXPECT_EQ(C.GetId(), G3_BB::C);
        EXPECT_EQ(D.GetId(), G3_BB::D);
        EXPECT_EQ(E.GetId(), G3_BB::E);
        EXPECT_EQ(F.GetId(), G3_BB::F);
        EXPECT_EQ(G.GetId(), G3_BB::G);
        EXPECT_EQ(H.GetId(), G3_BB::H);
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
    Allocator allocator_;

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

    Graph graph{GetAllocator(), &A, &D, 0};
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

/*
 ====================================================
 ==================== RPO tests =====================
 ====================================================
 */
TEST_F(GraphTest, Example1_Rpo) {
    using namespace G1_BB;

    Graph graph = GetFirstGraph();
    passes::Traversal tr{&graph};
    tr.Run();
    BlocksVector rpo = tr.getRPO();

    ASSERT_EQ(rpo.size(), 7);
    ASSERT_EQ(rpo.at(0)->GetId(), A);
    ASSERT_EQ(rpo.at(1)->GetId(), B);
    ASSERT_EQ(rpo.at(2)->GetId(), F);
    ASSERT_EQ(rpo.at(3)->GetId(), G);
    ASSERT_EQ(rpo.at(4)->GetId(), E);
    ASSERT_EQ(rpo.at(5)->GetId(), C);
    ASSERT_EQ(rpo.at(6)->GetId(), D);
}

TEST_F(GraphTest, Example2_Rpo) {
    using namespace G2_BB;

    Graph graph = GetSecondGraph();
    passes::Traversal tr{&graph};
    tr.Run();
    BlocksVector rpo = tr.getRPO();

    ASSERT_EQ(rpo.size(), 11);
    ASSERT_EQ(rpo.at(0)->GetId(), A);
    ASSERT_EQ(rpo.at(1)->GetId(), B);
    ASSERT_EQ(rpo.at(2)->GetId(), J);
    ASSERT_EQ(rpo.at(3)->GetId(), C);
    ASSERT_EQ(rpo.at(4)->GetId(), D);
    ASSERT_EQ(rpo.at(5)->GetId(), E);
    ASSERT_EQ(rpo.at(6)->GetId(), F);
    ASSERT_EQ(rpo.at(7)->GetId(), G);
    ASSERT_EQ(rpo.at(8)->GetId(), H);
    ASSERT_EQ(rpo.at(9)->GetId(), I);
    ASSERT_EQ(rpo.at(10)->GetId(), K);
}

TEST_F(GraphTest, Example3_Rpo) {
    using namespace G3_BB;

    Graph graph = GetThirdGraph();
    passes::Traversal tr{&graph};
    tr.Run();
    BlocksVector rpo = tr.getRPO();

    ASSERT_EQ(rpo.size(), 9);
    ASSERT_EQ(rpo.at(0)->GetId(), A);
    ASSERT_EQ(rpo.at(1)->GetId(), B);
    ASSERT_EQ(rpo.at(2)->GetId(), E);
    ASSERT_EQ(rpo.at(3)->GetId(), F);
    ASSERT_EQ(rpo.at(4)->GetId(), H);
    ASSERT_EQ(rpo.at(5)->GetId(), G);
    ASSERT_EQ(rpo.at(6)->GetId(), C);
    ASSERT_EQ(rpo.at(7)->GetId(), D);
    ASSERT_EQ(rpo.at(8)->GetId(), I);
}

/*
 ====================================================
 ================== DomTree tests ===================
 ====================================================
 */
template<typename...Args>
void TestBlockDominators(Graph *graph, uint8_t testId, Args...args) {
    std::vector<uint8_t> dominators{{args...}};
    auto bb_dom_blocks = graph->FindBlock(testId)->GetDomBlocks();
    ASSERT_EQ(bb_dom_blocks.size(), dominators.size());
    std::set<size_t> bbs_id;
    std::transform(bb_dom_blocks.begin(), bb_dom_blocks.end(),
                   std::inserter(bbs_id, bbs_id.end()), [](BasicBlock *bb) {
                return bb->GetId();
            });
    ASSERT_TRUE(std::all_of(dominators.begin(), dominators.end(), [&bbs_id](size_t id) {
        bool is_found = bbs_id.find(id) != bbs_id.end();
        EXPECT_TRUE(is_found) << "Error! BasicBlock with id = " << id << " wasn't found among dominators";
        return is_found;
    }));
}

TEST_F(GraphTest, Example_1_Dom_Tree) {
    using namespace G1_BB;
    Graph graph = GetFirstGraph();
    passes::DomTree domTree{&graph, true};
    ASSERT_TRUE(domTree.Run());

    {
        SCOPED_TRACE("A");
        // A dominators are: A
        TestBlockDominators(&graph, A, A);
        ASSERT_EQ(graph.FindBlock(A)->GetImmDom()->GetId(), A);
    }
    {
        SCOPED_TRACE("B");
        // B dominators are: A
        TestBlockDominators(&graph, B, A);
        ASSERT_EQ(graph.FindBlock(B)->GetImmDom()->GetId(), A);
    }
    {
        SCOPED_TRACE("C");
        // C dominators are: A, B
        TestBlockDominators(&graph, C, A, B);
        ASSERT_EQ(graph.FindBlock(C)->GetImmDom()->GetId(), B);
    }
    {
        SCOPED_TRACE("D");
        // D dominators are: A, B
        TestBlockDominators(&graph, D, A, B);
        ASSERT_EQ(graph.FindBlock(D)->GetImmDom()->GetId(), B);
    }
    {
        SCOPED_TRACE("E");
        // E dominators are: A, B, F
        TestBlockDominators(&graph, E, A, B, F);
        ASSERT_EQ(graph.FindBlock(E)->GetImmDom()->GetId(), F);
    }
    {
        SCOPED_TRACE("F");
        // F dominators are: A, B
        TestBlockDominators(&graph, F, A, B);
        ASSERT_EQ(graph.FindBlock(F)->GetImmDom()->GetId(), B);
    }
    {
        SCOPED_TRACE("G");
        // G dominators are: A, B, F
        TestBlockDominators(&graph, G, A, B, F);
        ASSERT_EQ(graph.FindBlock(G)->GetImmDom()->GetId(), F);
    }
}

TEST_F(GraphTest, Example2_Dom_Tree) {
    using namespace G2_BB;
    Graph graph = GetSecondGraph();
    passes::DomTree domTree{&graph, true};
    ASSERT_TRUE(domTree.Run());

    {
        SCOPED_TRACE("A");
        // A dominators are: A
        TestBlockDominators(&graph, A, A);
        ASSERT_EQ(graph.FindBlock(A)->GetImmDom()->GetId(), A);
    }
    {
        SCOPED_TRACE("B");
        // B dominators are: A
        TestBlockDominators(&graph, B, A);
        ASSERT_EQ(graph.FindBlock(B)->GetImmDom()->GetId(), A);
    }
    {
        SCOPED_TRACE("C");
        // C dominators are: A, B
        TestBlockDominators(&graph, C, A, B);
        ASSERT_EQ(graph.FindBlock(C)->GetImmDom()->GetId(), B);
    }
    {
        SCOPED_TRACE("D");
        // D dominators are: A, B, C
        TestBlockDominators(&graph, D, A, B, C);
        ASSERT_EQ(graph.FindBlock(D)->GetImmDom()->GetId(), C);
    }
    {
        SCOPED_TRACE("E");
        // E dominators are: A, B, C, D
        TestBlockDominators(&graph, E, A, B, C, D);
        ASSERT_EQ(graph.FindBlock(E)->GetImmDom()->GetId(), D);
    }
    {
        SCOPED_TRACE("F");
        // F dominators are: A, B, C, D
        TestBlockDominators(&graph, F, A, B, C, D, E);
        ASSERT_EQ(graph.FindBlock(F)->GetImmDom()->GetId(), E);
    }
    {
        SCOPED_TRACE("G");
        // G dominators are: A, B, C, D, E, F
        TestBlockDominators(&graph, G, A, B, C, D, E, F);
        ASSERT_EQ(graph.FindBlock(G)->GetImmDom()->GetId(), F);
    }
    {
        SCOPED_TRACE("H");
        // H dominators are: A, B, C, D, E, F, G
        TestBlockDominators(&graph, H, A, B, C, D, E, F, G);
        ASSERT_EQ(graph.FindBlock(H)->GetImmDom()->GetId(), G);
    }
    {
        SCOPED_TRACE("I");
        // I dominators are: A, B, C, D, E, F, G
        TestBlockDominators(&graph, I, A, B, C, D, E, F, G);
        ASSERT_EQ(graph.FindBlock(I)->GetImmDom()->GetId(), G);
    }
    {
        SCOPED_TRACE("J");
        // J dominators are: A, B
        TestBlockDominators(&graph, J, A, B);
        ASSERT_EQ(graph.FindBlock(J)->GetImmDom()->GetId(), B);
    }
    {
        SCOPED_TRACE("K");
        // K dominators are: A, B, C, D, E, F, G, I
        TestBlockDominators(&graph, K, A, B, C, D, E, F, G, I);
        ASSERT_EQ(graph.FindBlock(K)->GetImmDom()->GetId(), I);
    }
}

TEST_F(GraphTest, Example3_Dom_Tree) {
    using namespace G3_BB;
    Graph graph = GetThirdGraph();
    passes::DomTree domTree{&graph, true};
    ASSERT_TRUE(domTree.Run());

    {
        SCOPED_TRACE("A");
        // A dominators are: A
        TestBlockDominators(&graph, A, A);
        ASSERT_EQ(graph.FindBlock(A)->GetImmDom()->GetId(), A);
    }
    {
        SCOPED_TRACE("B");
        // B dominators are: A
        TestBlockDominators(&graph, B, A);
        ASSERT_EQ(graph.FindBlock(B)->GetImmDom()->GetId(), A);
    }
    {
        SCOPED_TRACE("C");
        // C dominators are: A, B
        TestBlockDominators(&graph, C, A, B);
        ASSERT_EQ(graph.FindBlock(C)->GetImmDom()->GetId(), B);
    }
    {
        SCOPED_TRACE("D");
        // D dominators are: A, B
        TestBlockDominators(&graph, D, A, B);
        ASSERT_EQ(graph.FindBlock(D)->GetImmDom()->GetId(), B);
    }
    {
        SCOPED_TRACE("E");
        // E dominators are: A, B
        TestBlockDominators(&graph, E, A, B);
        ASSERT_EQ(graph.FindBlock(E)->GetImmDom()->GetId(), B);
    }
    {
        SCOPED_TRACE("F");
        // F dominators are: A, B, E
        TestBlockDominators(&graph, F, A, B, E);
        ASSERT_EQ(graph.FindBlock(F)->GetImmDom()->GetId(), E);
    }
    {
        SCOPED_TRACE("G");
        // G dominators are: A, B
        TestBlockDominators(&graph, G, A, B);
        ASSERT_EQ(graph.FindBlock(G)->GetImmDom()->GetId(), B);
    }
    {
        SCOPED_TRACE("H");
        // H dominators are: A, B, E, F
        TestBlockDominators(&graph, H, A, B, E, F);
        ASSERT_EQ(graph.FindBlock(H)->GetImmDom()->GetId(), F);
    }
    {
        SCOPED_TRACE("I");
        // I dominators are: A, B
        TestBlockDominators(&graph, I, A, B);
        ASSERT_EQ(graph.FindBlock(I)->GetImmDom()->GetId(), B);
    }
}

/*
 =======================================================
 ================= LoopAnalyzer tests ==================
 =======================================================
 */
template<typename...Args>
void TestLoopBlocks(BasicBlock *header, uint8_t loop_id, Args...args) {
    std::vector<BasicBlock *> loop_blocks{{header->GetGraph()->FindBlock(args)...}};
    Loop *loop = header->GetLoop();
    auto loop_blocks_to_test = loop->GetLoopBlocks();
    ASSERT_EQ(loop->GetId(), loop_id);
    ASSERT_EQ(loop_blocks_to_test.size(), loop_blocks.size());
    std::set<size_t> bbs_id;
    std::transform(loop_blocks_to_test.begin(), loop_blocks_to_test.end(),
                   std::inserter(bbs_id, bbs_id.end()), [](BasicBlock *bb) {
                return bb->GetId();
            });
    ASSERT_TRUE(std::all_of(loop_blocks.begin(), loop_blocks.end(), [&bbs_id, loop_id](BasicBlock *bb) {
        bool is_found = bbs_id.find(bb->GetId()) != bbs_id.end();
        EXPECT_EQ(loop_id, bb->GetLoop()->GetId()) << "Error! BasicBlock with id = " << bb->GetId()
                                                   << " has wrong loop id = " << loop_id;
        EXPECT_TRUE(is_found) << "Error! BasicBlock with id = " << bb->GetId() << " wasn't found in loop " << loop_id;
        return is_found;
    }));
}

TEST_F(GraphTest, Example_1_Loop_Analyzer) {
    using namespace G1_BB;
    Graph graph = GetFirstGraph();
    passes::LoopAnalyzer loopAnalyzer{&graph};
    ASSERT_TRUE(loopAnalyzer.Run());
    ASSERT_TRUE(graph.IsLoopAnalysisValid());
    ASSERT_EQ(graph.GetRootLoop()->GetId(), 0);  // 1 loop in graph

    {
        // Root loop
        SCOPED_TRACE("A");
        BasicBlock *header = graph.FindBlock(A);
        // A loop blocks are: A, B, C, D, E, F, G
        TestLoopBlocks(header, 0, A, B, C, D, E, F, G);
        ASSERT_FALSE(header->GetLoop()->IsIrreducible());

        auto back_edges = header->GetLoop()->GetBackEdges();
        ASSERT_EQ(back_edges.size(), 0);
    }

}

TEST_F(GraphTest, Example_2_Loop_Analyzer) {
    using namespace G2_BB;
    Graph graph = GetSecondGraph();
    passes::LoopAnalyzer loopAnalyzer{&graph};
    ASSERT_TRUE(loopAnalyzer.Run());
    ASSERT_TRUE(graph.IsLoopAnalysisValid());
    ASSERT_EQ(graph.GetRootLoop()->GetId(), 3);  // 4 loops in graph

    {
        // Root loop
        SCOPED_TRACE("A");
        BasicBlock *header = graph.FindBlock(A);
        // A loop blocks are: A, I, K
        TestLoopBlocks(header, 3, A, I, K);
        ASSERT_FALSE(header->GetLoop()->IsIrreducible());

        auto back_edges = header->GetLoop()->GetBackEdges();
        ASSERT_EQ(back_edges.size(), 0);
    }
    {
        // Loop with header B
        SCOPED_TRACE("B");
        BasicBlock *header = graph.FindBlock(B);
        // B loop blocks are: B, J, G, H
        TestLoopBlocks(header, 0, B, J, G, H);
        ASSERT_FALSE(header->GetLoop()->IsIrreducible());

        auto back_edges = header->GetLoop()->GetBackEdges();
        ASSERT_EQ(back_edges.size(), 1);
        ASSERT_EQ(back_edges.at(0)->GetId(), H);
    }
    {
        // Loop with header E
        SCOPED_TRACE("E");
        BasicBlock *header = graph.FindBlock(E);
        // E loop blocks are: E, F
        TestLoopBlocks(header, 1, E, F);
        ASSERT_FALSE(header->GetLoop()->IsIrreducible());

        auto back_edges = header->GetLoop()->GetBackEdges();
        ASSERT_EQ(back_edges.size(), 1);
        ASSERT_EQ(back_edges.at(0)->GetId(), F);
    }
    {
        // Loop with header C
        SCOPED_TRACE("C");
        BasicBlock *header = graph.FindBlock(C);
        // C loop blocks are: C, D
        TestLoopBlocks(header, 2, C, D);
        ASSERT_FALSE(header->GetLoop()->IsIrreducible());

        auto back_edges = header->GetLoop()->GetBackEdges();
        ASSERT_EQ(back_edges.size(), 1);
        ASSERT_EQ(back_edges.at(0)->GetId(), D);
    }
}

TEST_F(GraphTest, Example_3_Loop_Analyzer) {
    using namespace G3_BB;
    Graph graph = GetThirdGraph();
    passes::LoopAnalyzer loopAnalyzer{&graph};
    ASSERT_TRUE(loopAnalyzer.Run());
    ASSERT_TRUE(graph.IsLoopAnalysisValid());
    ASSERT_EQ(graph.GetRootLoop()->GetId(), 2);  // 3 loops in graph

    {
        // Root loop
        SCOPED_TRACE("A");
        BasicBlock *header = graph.FindBlock(A);
        // A loop blocks are: A, H, D, I
        TestLoopBlocks(header, 2, A, H, D, I);
        ASSERT_FALSE(header->GetLoop()->IsIrreducible());

        auto back_edges = header->GetLoop()->GetBackEdges();
        ASSERT_EQ(back_edges.size(), 0);
    }
    {
        // Loop with header C
        SCOPED_TRACE("C");
        BasicBlock *header = graph.FindBlock(C);
        // C loop blocks are: C, G
        TestLoopBlocks(header, 0, C, G);
        ASSERT_TRUE(header->GetLoop()->IsIrreducible());

        auto back_edges = header->GetLoop()->GetBackEdges();
        ASSERT_EQ(back_edges.size(), 1);
        ASSERT_EQ(back_edges.at(0)->GetId(), G);
    }
    {
        // Loop with header B
        SCOPED_TRACE("B");
        BasicBlock *header = graph.FindBlock(B);
        // B loop blocks are: B, E, F
        TestLoopBlocks(header, 1, B, E, F);
        ASSERT_FALSE(header->GetLoop()->IsIrreducible());

        auto back_edges = header->GetLoop()->GetBackEdges();
        ASSERT_EQ(back_edges.size(), 1);
        ASSERT_EQ(back_edges.at(0)->GetId(), F);
    }
}

namespace G4_BB {
uint8_t A = 0, C = 1, B = 2, D = 3, E = 4;
}

TEST_F(GraphTest, Example_4_Loop_Analyzer) {
    /*
     *    `          A
     *               ↓
     *               B ←--------┐
     *             ↙   ↘        |
     *           C       D      |
     *                     ↘    |
     *                       E -┘
     *
     */
    BasicBlock A, B, C, D, E;

    BasicBlock::AddEdge(&A, &B);

    BasicBlock::AddEdge(&B, &C);
    BasicBlock::AddEdge(&B, &D);

    BasicBlock::AddEdge(&D, &E);

    BasicBlock::AddEdge(&E, &B);

    Graph graph{GetAllocator(), &A, &C, 0};
    graph.SetGraphForBasicBlocks({&A, &B, &C, &D, &E});

    EXPECT_EQ(A.GetId(), G4_BB::A);
    EXPECT_EQ(C.GetId(), G4_BB::C);
    EXPECT_EQ(B.GetId(), G4_BB::B);
    EXPECT_EQ(D.GetId(), G4_BB::D);
    EXPECT_EQ(E.GetId(), G4_BB::E);

    passes::LoopAnalyzer loopAnalyzer{&graph};
    ASSERT_TRUE(loopAnalyzer.Run());
    ASSERT_TRUE(graph.IsLoopAnalysisValid());
    ASSERT_EQ(graph.GetRootLoop()->GetId(), 1);  // 2 loops in graph

    {
        // Root loop
        SCOPED_TRACE("A");
        BasicBlock *header = graph.FindBlock(G4_BB::A);
        // A loop blocks are: A, C
        TestLoopBlocks(header, 1, G4_BB::A, G4_BB::C);
        ASSERT_FALSE(header->GetLoop()->IsIrreducible());

        auto back_edges = header->GetLoop()->GetBackEdges();
        ASSERT_EQ(back_edges.size(), 0);
    }
    {
        // Loop with header B
        SCOPED_TRACE("B");
        BasicBlock *header = graph.FindBlock(G4_BB::B);
        // B loop blocks are: B, D, E
        TestLoopBlocks(header, 0, G4_BB::B, G4_BB::D, G4_BB::E);
        ASSERT_FALSE(header->GetLoop()->IsIrreducible());

        auto back_edges = header->GetLoop()->GetBackEdges();
        ASSERT_EQ(back_edges.size(), 1);
        ASSERT_EQ(back_edges.at(0)->GetId(), G4_BB::E);
    }
}

namespace G5_BB {
uint8_t A = 0, F = 1, B = 2, C = 3, D = 4, E = 5;
}

TEST_F(GraphTest, Example_5_Loop_Analyzer) {
    /*
     *    `          A
     *               ↓
     *               B
     *             ↙   ↖
     *           C → D → E
     *             ↘ ↓
     *               F
     *
     */
    BasicBlock A, B, C, D, E, F;

    BasicBlock::AddEdge(&A, &B);

    BasicBlock::AddEdge(&B, &C);

    BasicBlock::AddEdge(&C, &F);
    BasicBlock::AddEdge(&C, &D);

    BasicBlock::AddEdge(&D, &F);
    BasicBlock::AddEdge(&D, &E);

    BasicBlock::AddEdge(&E, &B);

    Graph graph{GetAllocator(), &A, &F, 0};
    graph.SetGraphForBasicBlocks({&A, &B, &C, &D, &E, &F});

    EXPECT_EQ(A.GetId(), G5_BB::A);
    EXPECT_EQ(F.GetId(), G5_BB::F);
    EXPECT_EQ(B.GetId(), G5_BB::B);
    EXPECT_EQ(C.GetId(), G5_BB::C);
    EXPECT_EQ(D.GetId(), G5_BB::D);
    EXPECT_EQ(E.GetId(), G5_BB::E);

    passes::LoopAnalyzer loopAnalyzer{&graph};
    ASSERT_TRUE(loopAnalyzer.Run());
    ASSERT_TRUE(graph.IsLoopAnalysisValid());
    ASSERT_EQ(graph.GetRootLoop()->GetId(), 1);  // 2 loops in graph

    {
        // Root loop
        SCOPED_TRACE("A");
        BasicBlock *header = graph.FindBlock(G5_BB::A);
        // A loop blocks are: A, F
        TestLoopBlocks(header, 1, G5_BB::A, G5_BB::F);
        ASSERT_FALSE(header->GetLoop()->IsIrreducible());

        auto back_edges = header->GetLoop()->GetBackEdges();
        ASSERT_EQ(back_edges.size(), 0);
    }
    {
        // Loop with header B
        SCOPED_TRACE("B");
        BasicBlock *header = graph.FindBlock(G5_BB::B);
        // B loop blocks are: B, C, D, E
        TestLoopBlocks(header, 0, G5_BB::B, G5_BB::C, G5_BB::D, G5_BB::E);
        ASSERT_FALSE(header->GetLoop()->IsIrreducible());

        auto back_edges = header->GetLoop()->GetBackEdges();
        ASSERT_EQ(back_edges.size(), 1);
        ASSERT_EQ(back_edges.at(0)->GetId(), G5_BB::E);
    }
}

namespace G6_BB {
uint8_t A = 0, H = 1, B = 2, C = 3, D = 4, E = 5, F = 6, G = 7;
}

TEST_F(GraphTest, Example_6_Loop_Analyzer) {
    /*
     *    `          A ←---------┐
     *               ↓           |
     *               B ←-----┐   |
     *             ↙   ↘     |   |
     *           C       D   |   |
     *         ↙   ↘   ↙     |   |
     *       H       E       |   |
     *               ↓       |   |
     *               F ------┘   |
     *               ↓           |
     *               G ----------┘
     *
     */
    BasicBlock A, B, C, D, E, F, G, H;

    BasicBlock::AddEdge(&A, &B);

    BasicBlock::AddEdge(&B, &C);
    BasicBlock::AddEdge(&B, &D);

    BasicBlock::AddEdge(&C, &H);
    BasicBlock::AddEdge(&C, &E);

    BasicBlock::AddEdge(&D, &E);

    BasicBlock::AddEdge(&E, &F);

    BasicBlock::AddEdge(&F, &G);
    BasicBlock::AddEdge(&F, &B);

    BasicBlock::AddEdge(&G, &A);

    Graph graph{GetAllocator(), &A, &H, 0};
    graph.SetGraphForBasicBlocks({&A, &B, &C, &D, &E, &F, &G, &H});

    EXPECT_EQ(A.GetId(), G6_BB::A);
    EXPECT_EQ(H.GetId(), G6_BB::H);
    EXPECT_EQ(B.GetId(), G6_BB::B);
    EXPECT_EQ(C.GetId(), G6_BB::C);
    EXPECT_EQ(D.GetId(), G6_BB::D);
    EXPECT_EQ(E.GetId(), G6_BB::E);
    EXPECT_EQ(F.GetId(), G6_BB::F);
    EXPECT_EQ(G.GetId(), G6_BB::G);

    passes::LoopAnalyzer loopAnalyzer{&graph};
    ASSERT_TRUE(loopAnalyzer.Run());
    ASSERT_TRUE(graph.IsLoopAnalysisValid());
    ASSERT_EQ(graph.GetRootLoop()->GetId(), 2);  // 3 loops in graph

    {
        // Root loop
        SCOPED_TRACE("PreHeader");
        ASSERT_NE(graph.GetRootLoop()->GetPreHeader(), nullptr);
        BasicBlock *header = graph.GetRootLoop()->GetPreHeader();
        // PreHeader loop blocks are: PreHeader, H
        TestLoopBlocks(header, 2, header->GetId(), G6_BB::H);
        ASSERT_FALSE(header->GetLoop()->IsIrreducible());

        auto back_edges = header->GetLoop()->GetBackEdges();
        ASSERT_EQ(back_edges.size(), 0);
    }
    {
        // Loop with header A
        SCOPED_TRACE("A");
        BasicBlock *header = graph.FindBlock(G6_BB::A);
        // A loop blocks are: A, G
        TestLoopBlocks(header, 0, G6_BB::A, G6_BB::G);
        ASSERT_FALSE(header->GetLoop()->IsIrreducible());

        auto back_edges = header->GetLoop()->GetBackEdges();
        ASSERT_EQ(back_edges.size(), 1);
        ASSERT_EQ(back_edges.at(0)->GetId(), G6_BB::G);
    }
    {
        // Loop with header B
        SCOPED_TRACE("B");
        BasicBlock *header = graph.FindBlock(G6_BB::B);
        // B loop blocks are: B, C, D, E, F
        TestLoopBlocks(header, 1, G6_BB::B, G6_BB::C, G6_BB::D, G6_BB::E, G6_BB::F);
        ASSERT_FALSE(header->GetLoop()->IsIrreducible());

        auto back_edges = header->GetLoop()->GetBackEdges();
        ASSERT_EQ(back_edges.size(), 1);
        ASSERT_EQ(back_edges.at(0)->GetId(), G6_BB::F);
    }
}

}  // namespace compiler::test

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}