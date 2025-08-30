#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "config.h"
#include "utils/utils.h"
#include "utils/test_utils.h"
#include "map/subway.h"

class SubwayGraphTest : public ::testing::Test
{
protected:
    Transit::Map::Subway &map = Transit::Map::Subway::get_instance();
};

TEST_F(SubwayGraphTest, CreatesNodeStationsSuccessfully)
{
    using namespace Transit::Map;

    size_t n{10};
    std::vector<int> expected_ids{};

    try
    {
        expected_ids = TestUtils::extract_random_ids(std::string(DATA_DIRECTORY) + "/clean/subway/stations.csv", "complex_id", n);
    }
    catch (const std::runtime_error &e)
    {
        FAIL() << "In Subway tests, runtime error extracting random ids: " << e.what();
    }

    EXPECT_EQ(expected_ids.size(), n);

    for (const auto &id : expected_ids)
    {
        const Node *node{map.get_node(id)};
        EXPECT_NE(node, nullptr);
    }
}

TEST_F(SubwayGraphTest, CreatesConnectionsBetweenStationsSuccessfully)
{
    using namespace Transit::Map;

    std::vector<std::pair<int, int>> connected_pairs{
        {605, 147}, // 168 st and 175st, A line
        {609, 610}, // 42 - Bryand Park and Grand Central, 7 line
        {602, 405}, // Union Square and 23 st, 6 line
        {325, 326}  // Canal st and Franklin st, 1 line
    };

    const auto &adj_list{map.get_adjacency_list()};

    for (const auto &[u, v] : connected_pairs)
    {

        ASSERT_TRUE(adj_list.count(u)) << "Node " << u << " not found in adjacency list";
        ASSERT_TRUE(adj_list.count(v)) << "Node " << v << " not found in adjacency list";

        const auto &u_neighbors{adj_list.at(u)};
        const auto &v_neighbors{adj_list.at(v)};

        bool u_to_v = std::ranges::any_of(u_neighbors, [&](const Edge &edge)
                                          { return edge.to == v; });
        bool v_to_u = std::ranges::any_of(v_neighbors, [&](const Edge &edge)
                                          { return edge.to == u; });

        EXPECT_TRUE(u_to_v);
        EXPECT_TRUE(v_to_u);
    }
}

TEST_F(SubwayGraphTest, AddsVectorOfRoutesPerTrainLineSuccessfully)
{
    const auto &routes{map.get_routes()};

    for (const auto &[train_line, route_vec] : routes)
    {
        EXPECT_FALSE(route_vec.empty());
    }
}

TEST_F(SubwayGraphTest, FindsPathBetweenTwoStationsSuccessfully)
{
    using namespace Transit::Map;

    auto path_opt{map.find_path(384, 610)}; // burnside av, grand central
    ASSERT_TRUE(path_opt.has_value());

    auto path{*path_opt};
    EXPECT_FALSE(path.nodes.empty());
}