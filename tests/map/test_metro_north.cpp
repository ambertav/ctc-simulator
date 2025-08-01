#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <fstream>
#include <random>
#include <algorithm>

#include "config.h"
#include "utils.h"
#include "map/metro_north.h"

std::vector<int> extract_random_ids(const std::string &file_path, int count);

class MetroNorthGraphTest : public ::testing::Test
{
protected:
    Transit::Map::MetroNorth &map = Transit::Map::MetroNorth::get_instance();
};

TEST_F(MetroNorthGraphTest, CreatesNodeStationsSuccessfully)
{
    using namespace Transit::Map;

    int n{10};
    std::vector<int> expected_ids = extract_random_ids(std::string(DATA_DIRECTORY) + "/clean/mnr/stations.csv", n);
    EXPECT_EQ(expected_ids.size(), n);

    for (const auto &id : expected_ids)
    {
        const Node *node{map.get_node(id)};
        EXPECT_NE(node, nullptr);
    }
}

TEST_F(MetroNorthGraphTest, CreatesConnectionsBetweenStationsSuccessfully)
{
    using namespace Transit::Map;

    std::vector<std::pair<int, int>> connected_pairs{
        {42, 43},   // Garrison and Cold Spring, Hudson line
        {72, 74},   // Hartsdale and White Plains, Harlem line
        {124, 127}, // Stamford and Noroton Heights, New Haven line
        {154, 155}, // Springdale and Talmadge Hill, New Canaan branch
        {163, 164}, // Redding and Bethel, Danbury branch
        {169, 170}, // Seymour and Beacon Falls, Waterbury branch
    };

    auto adj_list{map.get_adjacency_list()};

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

TEST_F(MetroNorthGraphTest, AddsVectorOfRoutesPerTrainLineSuccessfully)
{
    const auto &routes{map.get_routes()};

    for (const auto &[train_line, route_vec] : routes)
    {
        EXPECT_FALSE(route_vec.empty());
    }
}

TEST_F(MetroNorthGraphTest, FindsPathBetweenTwoStationsSuccessfully)
{
    using namespace Transit::Map;

    auto path_opt{map.find_path(56, 114)}; // fordham, rye
    ASSERT_TRUE(path_opt.has_value());

    auto path{*path_opt};
    EXPECT_FALSE(path.nodes.empty());
}

std::vector<int> extract_random_ids(const std::string &file_path, int count)
{
    std::ifstream file(file_path);
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open station file");
    }

    std::string header{};
    std::getline(file, header);
    auto headers = Utils::split(header, ',');

    int id_index{-1};
    for (int i = 0; i < headers.size(); ++i)
    {
        if (headers[i] == "stop_id")
        {
            id_index = i;
            break;
        }
    }

    if (id_index == -1)
    {
        throw std::runtime_error("Missing stop_id column");
    }

    std::vector<int> all_ids{};
    std::string line{};

    while (std::getline(file, line))
    {
        auto tokens = Utils::split(line, ',');
        if (tokens.size() > id_index)
        {
            all_ids.push_back(Utils::string_view_to_numeric<int>(tokens[id_index]));
        }
    }

    std::shuffle(all_ids.begin(), all_ids.end(), std::mt19937{std::random_device{}()});
    all_ids.resize(std::min((int)all_ids.size(), count));

    return all_ids;
}