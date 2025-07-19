#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <fstream>
#include <random>
#include <algorithm>

#include "config.h"
#include "utils.h"
#include "map/subway.h"

std::vector<std::string> extract_random_complex_ids(const std::string &path, int count);

class SubwayGraphTest : public ::testing::Test
{
protected:
    Transit::Map::Subway &graph = Transit::Map::Subway::get_instance();
};

TEST_F(SubwayGraphTest, CreatesNodeStationsSuccessfully)
{
    using namespace Transit::Map;

    int n{10};
    std::vector<std::string> expected_ids = extract_random_complex_ids(std::string(DATA_DIRECTORY) + "/mta_subway_stations.csv", n);
    EXPECT_EQ(expected_ids.size(), n);

    for (const auto &id : expected_ids)
    {
        const Node *node = graph.get_node(id);
        EXPECT_NE(node, nullptr);
    }
}

TEST_F(SubwayGraphTest, CreatesConnectionsBetweenStationsSuccessfully)
{
    using namespace Transit::Map;

    std::vector<std::pair<std::string, std::string>> connected_pairs{
        {"605", "147"}, // 168 st and 175st, A line
        {"609", "610"}, // 42 - Bryand Park and Grand Central, 7 line
        {"602", "405"}, // Union Square and 23 st, 6 line
        {"325", "326"}  // Canal st and Franklin st, 1 line
    };

    auto adj_list = graph.get_adjacency_list();

    for (const auto &[u, v] : connected_pairs)
    {

        ASSERT_TRUE(adj_list.count(u)) << "Node " << u << "not found in adjacency list";
        ASSERT_TRUE(adj_list.count(v)) << "Node " << v << "not found in adjacency list";

        const auto &u_neighbors = adj_list.at(u);
        const auto &v_neighbors = adj_list.at(v);

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
    const auto &routes = graph.get_routes();

    for (const auto &[train_line, route_vec] : routes)
    {
        EXPECT_FALSE(route_vec.empty());
    }
}

TEST_F(SubwayGraphTest, FindsPathBetweenTwoStationsSuccessfully)
{
    using namespace Transit::Map;

    auto path_opt {graph.find_path("384", "610")}; // burnside av, grand central
    ASSERT_TRUE(path_opt.has_value());

    auto path {*path_opt};
    EXPECT_FALSE(path.nodes.empty());
}

std::vector<std::string> extract_random_complex_ids(const std::string &path, int count)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open station file");
    }

    std::string header;
    std::getline(file, header);
    auto headers = Utils::split(header, ',');

    int complex_index = -1;
    for (int i = 0; i < headers.size(); ++i)
    {
        if (headers[i] == "Complex ID")
        {
            complex_index = i;
            break;
        }
    }

    if (complex_index == -1)
    {
        throw std::runtime_error("Missing Complex ID column");
    }

    std::vector<std::string> all_ids;
    std::string line;
    while (std::getline(file, line))
    {
        auto tokens = Utils::split(line, ',');
        if (tokens.size() > complex_index)
        {
            all_ids.push_back(tokens[complex_index]);
        }
    }

    std::shuffle(all_ids.begin(), all_ids.end(), std::mt19937{std::random_device{}()});
    all_ids.resize(std::min((int)all_ids.size(), count));

    return all_ids;
}