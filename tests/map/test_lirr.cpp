#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <iostream>

#include "config.h"
#include "utils/utils.h"
#include "utils/test_utils.h"
#include "map/lirr.h"

class LongIslandGraphTest : public ::testing::Test
{
protected:
    Transit::Map::LongIslandRailroad &map = Transit::Map::LongIslandRailroad::get_instance();
};

TEST_F(LongIslandGraphTest, CreatesNodeStationsSucessfully)
{
    using namespace Transit::Map;

    int n{10};
    std::vector<int> expected_ids{};

    try
    {
        expected_ids = TestUtils::extract_random_ids(std::string(DATA_DIRECTORY) + "/clean/lirr/stations.csv", "stop_id", n);
    }
    catch (const std::runtime_error &e)
    {
        FAIL() << "In Long Island Railrod tests, runtime error extracting random ids: " << e.what();
    }

    EXPECT_EQ(expected_ids.size(), n);

    for (const auto &id : expected_ids)
    {
        const Node *node{map.get_node(id)};
        EXPECT_NE(node, nullptr);
    }
}

TEST_F(LongIslandGraphTest, CreatesConnectionsBetweenStationsSuccessfully)
{
    using namespace Transit::Map;

    std::vector<std::pair<int, int>> connected_pairs{
        {8, 38},    // Amityville and Copiague, Babylon
        {55, 214},  // Forest Hills and Woodside, City Terminal Zone
        {148, 50},  // Nostrand Ave and East New York, City Terminal Zone
        {211, 66},  // Valley Stream and Gibson, Far Rockaway
        {149, 68},  // Nassau Boulevard and Garden City, Hempstead
        {125, 31},  // Lynbrook and Centre Avenue, Long Beach
        {21, 140},  // Bellport and Mastic-Shirley, Montauk
        {185, 76},  // Sea Cliff and Glen Street, Oyster Bay
        {193, 14},  // St. James and Stony Brook, Port Jefferson
        {2, 25},    // Auburndale and Bayside, Port Washington
        {176, 126}, // Riverhead and Mattituck, Ronkonkoma
        {142, 124}  // Malverne and Lakeview, West Hempstead
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

TEST_F(LongIslandGraphTest, AddsVectorOfRoutesPerTrainLineSuccessfully)
{
    const auto &routes{map.get_routes()};

    for (const auto &[train_line, route_vec] : routes)
    {
        EXPECT_FALSE(route_vec.empty());
    }
}

TEST_F(LongIslandGraphTest, FindsPathBetweenTwoStationsSuccessfully)
{
    using namespace Transit::Map;

    auto path_opt{map.find_path(11, 141)}; // Broadway to Montauk (requires transfers)
    ASSERT_TRUE(path_opt.has_value());

    auto path{*path_opt};
    EXPECT_FALSE(path.nodes.empty());
}