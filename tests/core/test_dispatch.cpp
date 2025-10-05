#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <ranges>
#include <algorithm>

#include "utils/utils.h"
#include "constants/constants.h"
#include "system/registry.h"
#include "map/metro_north.h"
#include "core/agency_control.h"
#include "core/dispatch.h"

class DispatchTest : public ::testing::Test
{
protected:
    class MockAgencyControl : public AgencyControl
    {
        MockAgencyControl(Constants::System sc, const std::string &sn, const Transit::Map::Graph &g, const Registry &r) : AgencyControl(sc, sn, g, r) {}
    };

    std::unique_ptr<AgencyControl> ac{};
    Transit::Map::MetroNorth &mnr{Transit::Map::MetroNorth::get_instance()};
    Registry &registry{Registry::get_instance()};

    Dispatch *dispatch{};
    MNR::TrainLine train_line{MNR::TrainLine::HUDSON};

    void SetUp() override
    {
        Constants::System code{Constants::System::METRO_NORTH};
        auto it{std::ranges::find_if(Constants::SYSTEMS, [code](const auto &entry)
                                     { return entry.second == code; })};
        ASSERT_NE(it, Constants::SYSTEMS.end()) << "Invalid system code";

        ac = std::make_unique<AgencyControl>(code, it->first, mnr, registry);
        dispatch = ac->get_dispatch(train_line);
        ASSERT_NE(dispatch, nullptr);
    }
};

TEST_F(DispatchTest, ConstructorInitializesCorrectly)
{
    EXPECT_EQ(std::get<MNR::TrainLine>(dispatch->get_train_line()), train_line);

    const auto &stations{dispatch->get_stations()};
    EXPECT_THAT(stations, ::testing::Each(::testing::Truly([this](const auto &pair)
                                                           {
    const auto& [id, station] = pair;
    if (!station->is_yard()) {
        const Transit::Map::Node* node {mnr.get_node(id)};
        return node != nullptr && node->train_lines.contains(train_line);
    }
    return true; })));

    const auto &trains{dispatch->get_trains()};
    EXPECT_THAT(trains, ::testing::Each(::testing::Truly([this](const Train *t)
                                                         { return trainlines_equal(t->get_train_line(), train_line); })));
}

TEST_F(DispatchTest, LoadsScheduleSuccessfully)
{
    const auto &routes_map{mnr.get_routes()};
    const auto &schedules{dispatch->get_station_schedules()};

    auto it{routes_map.find(train_line)};
    ASSERT_NE(it, routes_map.end());

    const std::vector<Transit::Map::Route> &routes{it->second};
    size_t index{Utils::random_in_range(routes.size())};
    const auto &route{routes[index]};

    for (size_t i{0}; i < route.sequence.size(); ++i)
    {
        const Transit::Map::Node *node{mnr.get_node(route.sequence[i])};
        ASSERT_NE(node, nullptr);

        auto schedule_it{schedules.find(node->id)};
        ASSERT_NE(schedule_it, schedules.end());

        const auto &sched{schedule_it->second};
        EXPECT_FALSE(sched.arrivals.empty()) << "arrivals should have events in the queue";
        EXPECT_FALSE(sched.departures.empty()) << "departures should have events in the queue";

        int last_tick{-1};
        for (const auto &[tick, event] : sched.arrivals)
        {
            EXPECT_GT(tick, last_tick);
            last_tick = tick;
            EXPECT_EQ(event.station_id, node->id);
        }

        last_tick = -1;
        for (const auto &[tick, event] : sched.departures)
        {
            EXPECT_GT(tick, last_tick);
            last_tick = tick;
            EXPECT_EQ(event.station_id, node->id);
        }
    }
}

TEST_F(DispatchTest, AuthorizesTrainsCorrectly)
{
    int tick{0};
    dispatch->authorize(tick);
    const auto &authorizations_before_spawn{dispatch->get_authorizations()};
    EXPECT_TRUE(authorizations_before_spawn.empty()) << "dispatch should not authorize trains before spawning";

    dispatch->execute(tick); // spawns first train
    ++tick;

    dispatch->authorize(tick);
    const auto &authorizations{dispatch->get_authorizations()};
    EXPECT_FALSE(authorizations.empty()) << "dispatch should authorize train movement after spawn";

    Train *train{authorizations.front().first};
    Track *track{authorizations.front().second};

    Track *prev{track};
    Track *next{track->get_next_track(train_line)};
    while (next != nullptr && next->get_inbound_switch() == nullptr)
    {
        prev = next;
        next = next->get_next_track(train_line);
    }

    ASSERT_NE(next->get_inbound_switch(), nullptr) << "next track should have a switch";

    Signal *signal{prev->get_signal()};
    signal->change_state(SignalState::GREEN);
    train->move_to_track(prev);
    EXPECT_EQ(train->get_current_track(), prev) << "train should have moved into track right before the track with a switch";

    ++tick;
    dispatch->authorize(tick);
    const auto &sw_authorizations{dispatch->get_authorizations()};

    auto it{std::ranges::find_if(sw_authorizations.begin(), sw_authorizations.end(), [train](const std::pair<Train *, Track *> &pair)
                                 { return pair.first == train; })};
    EXPECT_EQ(it, sw_authorizations.end()) << "dispatch should not authorize train to move to track with switch";
}

TEST_F(DispatchTest, ExecutesTrainMovementAccordingToAuthorizations)
{
    int tick{0};
    dispatch->execute(tick);
    dispatch->authorize(tick);
    ++tick;
    
    Train *train{dispatch->get_authorizations().front().first};
    dispatch->execute(tick);
    ++tick;

    Track *current{train->get_current_track()};
    Track *next{current->get_next_track(train_line)};
    while (next != nullptr && next->get_inbound_switch() == nullptr)
    {
        current = next;
        next = next->get_next_track(train_line);
    }

    ASSERT_NE(next->get_inbound_switch(), nullptr) << "next track should have a switch";

    Signal *signal{current->get_signal()};
    signal->change_state(SignalState::GREEN);
    train->move_to_track(current);
    EXPECT_EQ(train->get_current_track(), current) << "train should have moved into track right before the track with a switch";

    while (train->get_dwell() > 0)  // have to decrement dwell
    {
        train->request_movement();
    }

    dispatch->authorize(tick);
    const auto &authorizations{dispatch->get_authorizations()};

    auto it{std::ranges::find_if(authorizations.begin(), authorizations.end(), [train](const std::pair<Train *, Track *> &pair)
                                 { return pair.first == train; })};
    EXPECT_EQ(it, authorizations.end()) << "dispatch should not authorize train to move to track with switch";

    ac->resolve_switches();
    
    dispatch->execute(tick);
    EXPECT_EQ(train->get_current_track(), next) << "agency control should authorize train and have it move into the track with a switch";
}