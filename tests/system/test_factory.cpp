#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <ranges>
#include <algorithm>

#include "map/metro_north.h"
#include "system/factory.h"

#include "utils/utils.h"
#include "utils/test_utils.h"
#include "constants/constants.h"

class FactoryTest : public ::testing::Test
{
protected:
    Factory factory{};
    Transit::Map::MetroNorth &graph{Transit::Map::MetroNorth::get_instance()};
    Registry &registry{Registry::get_instance()};
    Constants::System system_code{Constants::System::METRO_NORTH};

    void SetUp() override
    {
        factory.build_network(graph, registry, system_code);
    }
};

TEST_F(FactoryTest, CreatesTrainsSuccessfully)
{
    auto trains{factory.get_trains()};
    const auto &train_registry{registry.get_train_registry(system_code)};

    EXPECT_EQ(trains.size(), train_registry.size());

    std::vector<int> train_ids{};
    train_ids.reserve(trains.size());
    std::ranges::transform(trains, std::back_inserter(train_ids), [](const auto &train)
                           { return train->get_id(); });

    EXPECT_THAT(train_ids, ::testing::UnorderedElementsAreArray(train_registry));
}

TEST_F(FactoryTest, CreatesStationsSuccessfully)
{
    auto stations{factory.get_stations()};
    const auto &adj_list{graph.get_adjacency_list()};
    const auto &yard_registry{registry.get_yard_registry(system_code)};

    EXPECT_EQ(stations.size(), adj_list.size() + (yard_registry.size() * 2));

    size_t n{10};
    std::vector<int> expected_ids{TestUtils::extract_random_ids(graph, n)};
    EXPECT_EQ(expected_ids.size(), n);

    for (const auto &id : expected_ids)
    {
        auto it{std::find_if(stations.begin(), stations.end(), [&](Station *s)
                             { return s->get_id() == id; })};

        EXPECT_NE(it, stations.end()) << "Station " << id << " missing from Factory stations";
    }

    auto platforms {factory.get_platforms()};
    EXPECT_GE(platforms.size(), stations.size() * 2);
}

TEST_F(FactoryTest, CreatesConnectionsSuccessfully)
{
    auto stations{factory.get_stations()};
    const auto &routes_map{graph.get_routes()};

    TrainLine chosen_line{static_cast<MNR::TrainLine>(Utils::random_in_range(static_cast<std::size_t>(MNR::TrainLine::COUNT)))};
    const auto &routes{routes_map.at(chosen_line)};
    const auto &route{routes[Utils::random_in_range(routes.size())]};

    auto it{std::find_if(stations.begin(), stations.end(), [&](Station *s)
                         { return s->get_id() == route.sequence.front(); })};

    ASSERT_NE(it, stations.end());
    const Station *current_station{*it};

    for (int i{0}; i < route.sequence.size() - 1; ++i)
    {
        auto platform_opt{current_station->select_platform(route.direction, chosen_line)};
        ASSERT_TRUE(platform_opt.has_value());

        Platform *platform{*platform_opt};
        auto *next_track{platform->get_next_track(chosen_line)};
        ASSERT_NE(next_track, nullptr);

        Platform *next_platform{static_cast<Platform *>(next_track->get_next_track(chosen_line))};
        ASSERT_NE(next_platform, nullptr);

        const Station *next_station{next_platform->get_station()};
        EXPECT_EQ(next_station->get_id(), route.sequence[i + 1]);

        current_station = next_station;
    }
}

TEST_F(FactoryTest, CreatesAndConnectsSwitchesSuccessfully)
{
    auto switches{factory.get_switches()};
    ASSERT_FALSE(switches.empty());

    size_t count{std::min<size_t>(5, switches.size())};
    for (size_t i{0}; i < count; ++i)
    {
        Switch *sw{switches[i]};
        const auto &approach_tracks{sw->get_approach_tracks()};
        const auto &departure_tracks{sw->get_departure_tracks()};

        for (auto *tr : approach_tracks)
        {

            EXPECT_EQ(sw, tr->get_outbound_switch());
        }

        for (auto *tr : departure_tracks)
        {
            EXPECT_EQ(sw, tr->get_inbound_switch());
        }
    }
}