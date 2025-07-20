#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <ranges>
#include <algorithm>

#include "map/graph.h"
#include "core/factory.h"

#include "constants.h"

class FactoryTest : public ::testing::Test
{
protected:
    Factory factory;

    Transit::Map::Graph graph {};
    Transit::Map::Path path {};

    int number_of_trains{2};

    void SetUp() override
    {
        factory.clear();

        Transit::Map::Node *one {graph.add_node(1, "Station 1", {SUB::TrainLine::FOUR}, {"1"}, 4.0, 3.0)};
        Transit::Map::Node *two {graph.add_node(2, "Station 2", {SUB::TrainLine::FOUR}, {"2"}, 3.0, 2.0)};
        Transit::Map::Node *three {graph.add_node(3, "Station 3", {SUB::TrainLine::FOUR}, {"3"}, 2.0, 2.0)};

        graph.add_edge(1, 2);
        graph.add_edge(2, 3);

        auto path_opt{graph.find_path(1, 3)};
        ASSERT_TRUE(path_opt.has_value()) << "No path found in factory test";
        path = *path_opt;

        factory.build_network(number_of_trains, path);
    }
};

TEST_F(FactoryTest, TrainsWereCreatedSuccessfully)
{
    std::vector<Train *> trains = factory.get_trains();
    EXPECT_EQ(trains.size(), number_of_trains);
}

TEST_F(FactoryTest, StationsAndYardsCreatedSuccessfully)
{
    std::vector<Station *> stations = factory.get_stations();
    EXPECT_EQ(stations.size(), path.nodes.size() + 2);

    for (const auto &node : path.nodes)
    {
        auto it = std::find_if(stations.begin(), stations.end(), [&](Station *s)
                               { return s->get_id() == node->id; });

        EXPECT_NE(it, stations.end()) << "Station with ID " << node->id << " not found";
    }

    EXPECT_TRUE(std::ranges::any_of(stations, [](Station *s)
                                    { return s->get_id() == Yards::ids[0]; }))
        << "North yard not found";

    EXPECT_TRUE(std::ranges::any_of(stations, [](Station *s)
                                    { return s->get_id() == Yards::ids[1]; }))
        << "South yard not found";
}

TEST_F(FactoryTest, SignalsCreatedSuccessfully)
{
    std::vector<Platform *> platforms = factory.get_platforms();
    std::vector<Track *> tracks = factory.get_tracks();

    std::vector<Signal *> signals = factory.get_signals();

    EXPECT_EQ(signals.size(), platforms.size() + tracks.size());

    for (Platform *p : platforms)
    {
        EXPECT_NE(p->get_signal(), nullptr);
    }

    for (Track *t : tracks)
    {
        EXPECT_NE(t->get_signal(), nullptr);
    }
}

TEST_F(FactoryTest, PlatformsCreatedSuccessfully)
{
    int expected_platforms_per_direction = path.nodes.size() + 2;
    int expected_total_platforms = expected_platforms_per_direction * 2;

    std::vector<Platform *> platforms = factory.get_platforms();

    EXPECT_EQ(platforms.size(), expected_total_platforms);

    std::vector<Direction> directions {SUB::Direction::DOWNTOWN, SUB::Direction::UPTOWN};

    std::vector<Station *> stations = factory.get_stations();
    for (Direction dir : directions)
    {
        int platform_count = std::accumulate(stations.begin(), stations.end(), 0, [dir](int sum, Station *station)
                                             { return sum + station->get_platforms_by_direction(dir).size(); });

        EXPECT_EQ(platform_count, expected_platforms_per_direction) << "Wrong platform count for direction " << testing::PrintToString(dir);
    }
}

TEST_F(FactoryTest, TracksCreatedSuccessfully)
{
    int expected_tracks_per_direction = path.nodes.size() + 1;
    int expected_total_tracks = expected_tracks_per_direction * 2;

    std::vector<Track *> tracks = factory.get_tracks();

    EXPECT_EQ(tracks.size(), expected_total_tracks);
}

// TEST_F(FactoryTest, ConnectsTracksAndPlatformsSuccessfully)
// {
//     std::vector<Station *> stations = factory.get_stations();

//     Station *north = nullptr;
//     Station *south = nullptr;

//     for (Station *s : stations)
//     {
//         if (s->get_id() == Yards::north)
//         {
//             north = s;
//         }

//         if (s->get_id() == Yards::south)
//         {
//             south = s;
//         }
//     }

//     ASSERT_NE(north, nullptr) << "North yard is nullptr";
//     ASSERT_NE(south, nullptr) << "South yard is nullptr";

//     for (auto [station, direction] : {
//              std::make_pair(north, SUB::Direction::DOWNTOWN),
//              std::make_pair(south, SUB::Direction::UPTOWN)})
//     {
//         auto platforms = station->get_platforms_by_direction(direction);
//         ASSERT_FALSE(platforms.empty()) << "No platforms for " << direction;

//         Platform *start_platform = platforms.front();
//         Track *current = start_platform;
//         Track *prev = nullptr;

//         bool expect_platform = false;
//         int step = 0;

//         while (current)
//         {
//             if (prev)
//             {
//                 EXPECT_EQ(current->get_prev(), prev) << "Broken prev link at step: " << step;
//             }

//             if (step > 0)
//             {
//                 EXPECT_EQ(current->is_platform(), expect_platform) << "Expected " << (expect_platform ? "Platform" : "Track") << " at step: " << step;
//                 expect_platform = !expect_platform;
//             }

//             prev = current;
//             current = current->get_next();
//             ASSERT_LT(++step, 100) << "Infinite loop detected";
//         }

//         EXPECT_TRUE(prev->is_platform()) << "Path should end at platform";
//     }
// }

TEST_F(FactoryTest, ClearsObjectsSuccessfully)
{
    factory.clear();

    EXPECT_TRUE(factory.get_trains().empty());
    EXPECT_TRUE(factory.get_stations().empty());
    EXPECT_TRUE(factory.get_signals().empty());
    EXPECT_TRUE(factory.get_platforms().empty());
    EXPECT_TRUE(factory.get_tracks().empty());
}