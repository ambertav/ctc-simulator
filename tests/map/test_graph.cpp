#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "map/graph.h"

class GraphTest : public ::testing::Test
{
protected:
    Transit::Map::Graph graph;
    std::unordered_map<char, int> name_to_id{
        {'A', 1},
        {'B', 2},
        {'C', 3},
        {'D', 4},
        {'E', 5}};

    Transit::Map::Node *A;
    Transit::Map::Node *B;
    Transit::Map::Node *C;
    Transit::Map::Node *D;
    Transit::Map::Node *E;

    double AB_weight = 3.0;
    double BC_weight = 2.0;
    double AE_weight = AB_weight;
    double EC_weight = BC_weight;

    void SetUp() override
    {
        using namespace Transit::Map;

        A = graph.add_node(name_to_id['A'], "Station A", {SUB::TrainLine::A}, {"1"});
        B = graph.add_node(name_to_id['B'], "Station B", {SUB::TrainLine::A}, {"2"});
        C = graph.add_node(name_to_id['C'], "Station C", {SUB::TrainLine::A}, {"3"});
        D = graph.add_node(name_to_id['D'], "Station D", {SUB::TrainLine::A}, {"4"});
        E = graph.add_node(name_to_id['E'], "Station E", {SUB::TrainLine::B}, {"5"});

        graph.add_edge(A, B, AB_weight, {SUB::TrainLine::A});
        graph.add_edge(B, C, BC_weight, {SUB::TrainLine::A});
        graph.add_edge(A, E, AE_weight, {SUB::TrainLine::B});
        graph.add_edge(E, C, EC_weight, {SUB::TrainLine::B});
    }
};

TEST_F(GraphTest, CreatedAndAddedNodesSuccessfully)
{
    EXPECT_EQ(A->id, name_to_id['A']);
    EXPECT_EQ(B->id, name_to_id['B']);
    EXPECT_EQ(C->id, name_to_id['C']);

    const auto &adj_list = graph.get_adjacency_list();

    EXPECT_EQ(adj_list.count(name_to_id['A']), 1);
    EXPECT_EQ(adj_list.count(name_to_id['B']), 1);
    EXPECT_EQ(adj_list.count(name_to_id['C']), 1);
}

TEST_F(GraphTest, DuplicateNodeInsertionFails)
{
    EXPECT_THROW(graph.add_node(name_to_id['A'], "Duplicate Station", {SUB::TrainLine::A}, {"4"}), std::invalid_argument);
}

TEST_F(GraphTest, EdgesCreatedSuccessfully)
{
    using namespace Transit::Map;

    const auto &adj_list = graph.get_adjacency_list();

    const auto &edges_from_A = adj_list.at(name_to_id['A']);
    const auto &edges_from_B = adj_list.at(name_to_id['B']);

    auto it_AB = std::find_if(edges_from_A.begin(), edges_from_A.end(), [&](const Edge &e)
                              { return e.to == name_to_id['B']; });
    EXPECT_NE(it_AB, edges_from_A.end());
    EXPECT_EQ(it_AB->weight, AB_weight);

    auto it_BA = std::find_if(edges_from_B.begin(), edges_from_B.end(), [&](const Edge &e)
                              { return e.to == name_to_id['A']; });
    EXPECT_NE(it_BA, edges_from_B.end());
    EXPECT_EQ(it_BA->weight, AB_weight);
}

TEST_F(GraphTest, RemovesEdgeSuccessfully)
{
    using namespace Transit::Map;

    graph.remove_edge(A, B);

    const auto &adj_list = graph.get_adjacency_list();

    const auto &edges_from_A = adj_list.at(name_to_id['A']);
    const auto &edges_from_B = adj_list.at(name_to_id['B']);

    auto it_AB = std::find_if(edges_from_A.begin(), edges_from_A.end(), [&](const Edge &e)
                              { return e.to == name_to_id['B']; });
    EXPECT_EQ(it_AB, edges_from_A.end());

    auto it_BA = std::find_if(edges_from_B.begin(), edges_from_B.end(), [&](const Edge &e)
                              { return e.to == name_to_id['A']; });
    EXPECT_EQ(it_BA, edges_from_B.end());
}

TEST_F(GraphTest, DeletesNodeAndAllCorrespondingEdges)
{
    using namespace Transit::Map;
    graph.remove_node(B);

    const auto &adj_list = graph.get_adjacency_list();

    EXPECT_EQ(adj_list.count(name_to_id['B']), 0);

    EXPECT_EQ(adj_list.count(name_to_id['A']), 1);
    EXPECT_EQ(adj_list.count(name_to_id['C']), 1);

    const auto &edges_from_A = adj_list.at(name_to_id['A']);
    const auto &edges_from_C = adj_list.at(name_to_id['C']);

    auto it_AB = std::find_if(edges_from_A.begin(), edges_from_A.end(), [&](const Edge &e)
                              { return e.to == name_to_id['B']; });
    EXPECT_EQ(it_AB, edges_from_A.end());

    auto it_CB = std::find_if(edges_from_C.begin(), edges_from_C.end(), [&](const Edge &e)
                              { return e.to == name_to_id['B']; });
    EXPECT_EQ(it_CB, edges_from_C.end());
}

TEST_F(GraphTest, FindsPathSuccessfully)
{
    auto path_opt = graph.find_path(name_to_id['A'], name_to_id['C']);
    ASSERT_TRUE(path_opt.has_value()) << "No path found in graph test";
    auto path{*path_opt};

    EXPECT_EQ(path.nodes, std::vector<const Transit::Map::Node *>({A, B, C}));
    EXPECT_EQ(path.segment_weights, std::vector<double>({AB_weight, BC_weight}));
    EXPECT_EQ(path.total_weight, AB_weight + BC_weight);
}

TEST_F(GraphTest, FindPathHandlesCycles)
{
    // A->B->C vs A->C path
    graph.add_edge(C, A, 1, {SUB::TrainLine::A});

    auto path_opt = graph.find_path(name_to_id['A'], name_to_id['C']);
    ASSERT_TRUE(path_opt.has_value()) << "No path found in graph test";
    auto path{*path_opt};

    EXPECT_EQ(path.nodes, std::vector<const Transit::Map::Node *>({A, C}));
}

TEST_F(GraphTest, FindsLeastTransfersPathForMultipleShortestPaths)
{
    // A->B->C and A->E->C have equal weight
    // A->B->C has less transfers
    // should return A->B->C

    auto path_opt = graph.find_path(name_to_id['A'], name_to_id['C']);
    ASSERT_TRUE(path_opt.has_value()) << "No path found in graph test";
    auto path{*path_opt};

    EXPECT_EQ(path.total_weight, AB_weight + BC_weight);
    EXPECT_EQ(path.nodes, std::vector<const Transit::Map::Node *>({A, B, C}));
}

TEST_F(GraphTest, PathToSelfReturnsEmptyPath)
{
    auto path_opt = graph.find_path(name_to_id['A'], name_to_id['A']);
    ASSERT_FALSE(path_opt.has_value()) << "Path to self was found";
}

TEST_F(GraphTest, NoPathReturnsEmptyPath)
{
    auto path_opt = graph.find_path(name_to_id['A'], name_to_id['D']);
    ASSERT_FALSE(path_opt.has_value()) << "Path was found when there should not be";
}