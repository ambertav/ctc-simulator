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

    int AB_weight = 3;
    int BC_weight = 2;

    void SetUp() override
    {
        using namespace Transit::Map;

        A = graph.add_node("A", "Station A", {TrainLine::A}, ServiceType::BOTH);
        B = graph.add_node("B", "Station B", {TrainLine::A}, ServiceType::BOTH);
        C = graph.add_node("C", "Station C", {TrainLine::A}, ServiceType::BOTH);

        graph.add_edge(A, B, AB_weight, {TrainLine::A}, ServiceType::BOTH);
        graph.add_edge(B, C, BC_weight, {TrainLine::A}, ServiceType::BOTH);
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
    EXPECT_THROW(graph.add_node("A", "Duplicate Station", {TrainLine::A}, ServiceType::BOTH), std::invalid_argument);
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