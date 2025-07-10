#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "map/base.h"

class GraphTest : public ::testing::Test
{
protected:
    Transit::Map::Base graph;

    Transit::Map::Node *A;
    Transit::Map::Node *B;
    Transit::Map::Node *C;
    Transit::Map::Node *D;
    Transit::Map::Node *E;

    int AB_weight = 3;
    int BC_weight = 2;
    int AE_weight = AB_weight;
    int EC_weight = BC_weight;

    void SetUp() override
    {
        using namespace Transit::Map;

        A = graph.add_node("A", "Station A", {TrainLine::A}, {"1"});
        B = graph.add_node("B", "Station B", {TrainLine::A}, {"2"});
        C = graph.add_node("C", "Station C", {TrainLine::A}, {"3"});
        D = graph.add_node("D", "Station D", {TrainLine::A}, {"4"});
        E = graph.add_node("E", "Station E", {TrainLine::B}, {"5"});

        graph.add_edge(A, B, AB_weight, {TrainLine::A});
        graph.add_edge(B, C, BC_weight, {TrainLine::A});
        graph.add_edge(A, E, AE_weight, {TrainLine::B});
        graph.add_edge(E, C, EC_weight, {TrainLine::B});
    }
};

TEST_F(GraphTest, CreatedAndAddedNodesSuccessfully)
{
    EXPECT_EQ(A->id, "A");
    EXPECT_EQ(B->id, "B");
    EXPECT_EQ(C->id, "C");

    const auto &adj_list = graph.get_adjacency_list();

    EXPECT_EQ(adj_list.count("A"), 1);
    EXPECT_EQ(adj_list.count("B"), 1);
    EXPECT_EQ(adj_list.count("C"), 1);
}

TEST_F(GraphTest, DuplicateNodeInsertionFails)
{
    EXPECT_THROW(graph.add_node("A", "Duplicate Station", {TrainLine::A}, {"4"}), std::invalid_argument);
}

TEST_F(GraphTest, EdgesCreatedSuccessfully)
{
    using namespace Transit::Map;

    const auto &adj_list = graph.get_adjacency_list();

    const auto &edges_from_A = adj_list.at("A");
    const auto &edges_from_B = adj_list.at("B");

    auto it_AB = std::find_if(edges_from_A.begin(), edges_from_A.end(), [](const Edge &e)
                              { return e.to == "B"; });
    EXPECT_NE(it_AB, edges_from_A.end());
    EXPECT_EQ(it_AB->weight, AB_weight);

    auto it_BA = std::find_if(edges_from_B.begin(), edges_from_B.end(), [](const Edge &e)
                              { return e.to == "A"; });
    EXPECT_NE(it_BA, edges_from_B.end());
    EXPECT_EQ(it_BA->weight, AB_weight);
}

TEST_F(GraphTest, RemovesEdgeSuccessfully)
{
    using namespace Transit::Map;

    graph.remove_edge(A, B);

    const auto &adj_list = graph.get_adjacency_list();

    const auto &edges_from_A = adj_list.at("A");
    const auto &edges_from_B = adj_list.at("B");

    auto it_AB = std::find_if(edges_from_A.begin(), edges_from_A.end(), [](const Edge &e)
                              { return e.to == "B"; });
    EXPECT_EQ(it_AB, edges_from_A.end());

    auto it_BA = std::find_if(edges_from_B.begin(), edges_from_B.end(), [](const Edge &e)
                              { return e.to == "A"; });
    EXPECT_EQ(it_BA, edges_from_B.end());
}

TEST_F(GraphTest, DeletesNodeAndAllCorrespondingEdges)
{
    using namespace Transit::Map;
    graph.remove_node(B);

    const auto &adj_list = graph.get_adjacency_list();

    EXPECT_EQ(adj_list.count("B"), 0);

    EXPECT_EQ(adj_list.count("A"), 1);
    EXPECT_EQ(adj_list.count("C"), 1);

    const auto &edges_from_A = adj_list.at("A");
    const auto &edges_from_C = adj_list.at("C");

    auto it_AB = std::find_if(edges_from_A.begin(), edges_from_A.end(), [](const Edge &e)
                              { return e.to == "B"; });
    EXPECT_EQ(it_AB, edges_from_A.end());

    auto it_CB = std::find_if(edges_from_C.begin(), edges_from_C.end(), [](const Edge &e)
                              { return e.to == "B"; });
    EXPECT_EQ(it_CB, edges_from_C.end());
}

TEST_F(GraphTest, FindsPathSuccessfully)
{
    auto path = graph.find_path("A", "C");

    EXPECT_TRUE(path.path_found);
    EXPECT_EQ(path.path, std::vector<const Transit::Map::Node *>({A, B, C}));
    EXPECT_EQ(path.segment_weights, std::vector<int>({AB_weight, BC_weight}));
    EXPECT_EQ(path.total_weight, AB_weight + BC_weight);
}

TEST_F(GraphTest, FindPathHandlesCycles)
{
    // A->B->C vs A->C path
    graph.add_edge(C, A, 1, {TrainLine::A});
    auto path = graph.find_path("A", "C");

    EXPECT_TRUE(path.path_found);

    EXPECT_EQ(path.path, std::vector<const Transit::Map::Node *>({A, C}));
}

TEST_F(GraphTest, FindsLeastTransfersPathForMultipleShortestPaths)
{
    // A->B->C and A->E->C have equal weight
    // A->B->C has less transfers
    // should return A->B->C

    auto path = graph.find_path("A", "C");

    EXPECT_TRUE(path.path_found);
    EXPECT_EQ(path.total_weight, AB_weight + BC_weight);

    EXPECT_EQ(path.path, std::vector<const Transit::Map::Node *>({A, B, C}));
}

TEST_F(GraphTest, PathToSelfReturnsEmptyPath)
{
    auto path = graph.find_path("A", "A");

    EXPECT_FALSE(path.path_found);
    EXPECT_TRUE(path.path.empty());
    EXPECT_TRUE(path.segment_weights.empty());
    EXPECT_EQ(path.total_weight, 0);
}

TEST_F(GraphTest, NoPathReturnsEmptyPath)
{
    auto path = graph.find_path("A", "D");

    EXPECT_FALSE(path.path_found);
    EXPECT_TRUE(path.path.empty());
    EXPECT_TRUE(path.segment_weights.empty());
    EXPECT_EQ(path.total_weight, 0);
}