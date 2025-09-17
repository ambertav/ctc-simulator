#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <ranges>
#include <algorithm>

#include "utils/utils.h"
#include "system/registry.h"
#include "map/metro_north.h"
#include "core/dispatch.h"
#include "core/central_control.h"

class CentralControlTest : public ::testing::Test
{
protected:
    std::unique_ptr<CentralControl> central_control{};
    Transit::Map::MetroNorth &mnr{Transit::Map::MetroNorth::get_instance()};
    Registry &registry{Registry::get_instance()};
    Dispatch *dispatch{};

    std::string name{};
    Constants::System code{};

    Station *to_inbound{};
    Station *to_outbound{};
    TrainLine train_line{MNR::TrainLine::NEW_HAVEN};

    void SetUp() override
    {
        code = Constants::System::METRO_NORTH;
        auto it{std::ranges::find_if(Constants::SYSTEMS, [&](const auto &entry)
                                     { return entry.second == code; })};
        ASSERT_NE(it, Constants::SYSTEMS.end()) << "Invalid system code";
        name = it->first;

        central_control = std::make_unique<CentralControl>(code, name, mnr, registry);
        dispatch = central_control->get_dispatch(train_line);
        ASSERT_NE(dispatch, nullptr);

        const auto &yard_registry{registry.get_yard_registry(code)};
        ASSERT_FALSE(yard_registry.empty());

        auto pair_it{std::ranges::find_if(yard_registry, [&](const auto &pair)
                                          { 
                                                Info yard_info{registry.decode(pair.first)};
                                                return trainlines_equal(yard_info.train_line, train_line); })};

        ASSERT_NE(pair_it, yard_registry.end());

        const auto &stations{dispatch->get_stations()};
        auto in_it{stations.find(pair_it->first)};
        auto out_it{stations.find(pair_it->second)};

        ASSERT_NE(in_it, stations.end());
        ASSERT_NE(out_it, stations.end());

        to_inbound = out_it->second;
        to_outbound = in_it->second;
    }

    std::tuple<Train *, Switch *, Track * /* from */, Track * /* to */> switch_scenario(size_t index, MNR::Direction direction)
    {
        const auto &trains{dispatch->get_trains()};
        EXPECT_FALSE(trains.empty());
        EXPECT_LT(index, trains.size());

        Train *train{trains[index]};

        Station *yard{};
        if (directions_equal(MNR::Direction::INBOUND, direction))
        {
            yard = to_inbound;
        }
        else
        {
            yard = to_outbound;
        }

        std::optional<Platform *> platform_opt{yard->select_platform(direction, train_line)};
        EXPECT_TRUE(platform_opt.has_value());

        Platform *platform{*platform_opt};
        EXPECT_NE(platform, nullptr);
        train->spawn(platform);

        Track *prev{static_cast<Track *>(platform)};
        Track *next{prev->get_next_track(train_line)};
        while (next != nullptr && next->get_inbound_switch() == nullptr)
        {
            prev = next;
            next = next->get_next_track(train_line);
        }

        Switch *sw{next->get_inbound_switch()};
        EXPECT_NE(sw, nullptr) << "next track should have a switch";

        Signal *signal{prev->get_signal()};
        signal->change_state(SignalState::GREEN);
        train->move_to_track(prev);

        return {train, sw, prev, next};
    }
};

TEST_F(CentralControlTest, ConstructorInitializesCorrectly)
{
    EXPECT_NE(central_control, nullptr);
    EXPECT_EQ(central_control->get_system_name(), name);

    for (int i{0}; i < static_cast<int>(MNR::TrainLine::COUNT); ++i)
    {
        auto line{static_cast<MNR::TrainLine>(i)};
        Dispatch *valid_dispatch{central_control->get_dispatch(line)};
        EXPECT_NE(valid_dispatch, nullptr) << "dispatch should be created for train line";

        EXPECT_TRUE(trainlines_equal(valid_dispatch->get_train_line(), line));
        EXPECT_FALSE(valid_dispatch->get_trains().empty());
        EXPECT_FALSE(valid_dispatch->get_stations().empty());
    }

    Dispatch *invalid_dispatch{central_control->get_dispatch(SUB::TrainLine::A)};
    EXPECT_EQ(invalid_dispatch, nullptr);
}

TEST_F(CentralControlTest, HandlesSwitchRequestAndResolveForDifferentSwitches)
{
    const auto &trains{dispatch->get_trains()};
    EXPECT_FALSE(trains.empty());

    size_t first_index{Utils::random_index(trains.size())};
    size_t second_index{Utils::random_index(trains.size())};
    while (second_index == first_index)
    {
        second_index = Utils::random_index(trains.size());
    }
    ASSERT_NE(first_index, second_index);

    auto [train_1, sw_1, from_1, to_1] = switch_scenario(first_index, MNR::Direction::OUTBOUND);
    auto [train_2, sw_2, from_2, to_2] = switch_scenario(second_index, MNR::Direction::INBOUND);

    ASSERT_NE(sw_1, sw_2) << "the trains should be requesting access for different switches";

    int tick{100};
    int priority{5};

    central_control->request_switch(train_1, sw_1, from_1, to_1, priority, tick, dispatch);
    central_control->request_switch(train_2, sw_2, from_2, to_2, priority, tick, dispatch);

    auto empty_granted_links{central_control->get_granted_links(dispatch)};
    EXPECT_TRUE(empty_granted_links.empty());

    central_control->resolve_switches();
    auto granted_links{central_control->get_granted_links(dispatch)};
    EXPECT_FALSE(granted_links.empty());
}

TEST_F(CentralControlTest, HandlesSwitchRequestAndResolveForCollisions)
{
    const auto &trains{dispatch->get_trains()};
    EXPECT_FALSE(trains.empty());

    size_t first_index{Utils::random_index(trains.size())};
    size_t second_index{Utils::random_index(trains.size())};
    while (second_index == first_index)
    {
        second_index = Utils::random_index(trains.size());
    }
    ASSERT_NE(first_index, second_index);

    auto [train_1, sw_1, from_1, to_1] = switch_scenario(first_index, MNR::Direction::OUTBOUND);
    auto [train_2, sw_2, from_2, to_2] = switch_scenario(second_index, MNR::Direction::OUTBOUND);

    ASSERT_EQ(sw_1, sw_2) << "both trains should be requesting access to same switch";

    int tick{100};
    int first_priority{10};
    int second_priority{5};

    central_control->request_switch(train_1, sw_1, from_1, to_1, first_priority, tick, dispatch);
    central_control->request_switch(train_2, sw_2, from_2, to_2, second_priority, tick, dispatch);

    auto empty_granted_links{central_control->get_granted_links(dispatch)};
    EXPECT_TRUE(empty_granted_links.empty());

    central_control->resolve_switches();
    auto granted_links{central_control->get_granted_links(dispatch)};
    EXPECT_EQ(granted_links.size(), 1) << "only one train should be granted the switch";
    EXPECT_EQ(granted_links.front().first, train_1);
    EXPECT_EQ(granted_links.front().second, to_1);
}