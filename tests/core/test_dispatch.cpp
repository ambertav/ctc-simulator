#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

#include "config.h"
#include "core/dispatch.h"

class DispatchTest : public testing::Test
{
protected:
    class MockTrain : public Train
    {
    public:
        MockTrain(int i, TrainLine l, ServiceType t, Track *ct) : Train(i, l, t, ct) {}
    };
    class MockTrack : public Track
    {
    public:
        MockTrack(int i, Signal *s, int d = 1) : Track(i, s, d) {}
    };
    class MockPlatform : public Platform
    {
    public:
        MockPlatform(int i, int dw, Signal *si, const Station *st, Direction dir) : Platform(i, dw, si, st, dir) {}
    };
    class MockSignal : public Signal
    {
    public:
        MockSignal(int i, int t) : Signal(i, t) {}
    };
    class MockStation : public Station
    {
    public:
        MockStation(int i, const std::string &n, bool y, const std::vector<TrainLine> &l) : Station(i, n, y, l) {}
    };

    std::unique_ptr<MockTrain> mock_train_1;
    std::unique_ptr<MockTrain> mock_train_2;
    std::unique_ptr<MockStation> mock_yard_1, mock_station, mock_yard_2;
    std::unique_ptr<MockPlatform> mock_platform_yard_1, mock_platform_station, mock_platform_yard_2;
    std::unique_ptr<MockTrack> mock_track_1, mock_track_2;
    std::unique_ptr<MockSignal> mock_signal_yard_1, mock_signal_track_1, mock_signal_station, mock_signal_track_2, mock_signal_yard_2;
    std::unique_ptr<Dispatch> dispatch;

    std::string test_file{std::string(LOG_DIRECTORY) + "/test_sim.txt"};
    std::string test_schedule{std::string(DATA_DIRECTORY) + "/dispatch_test_schedule.csv"};
    Logger logger{test_file};

    void build_objects()
    {
        auto lines = std::vector<TrainLine>{TrainLine::FOUR};

        mock_train_1 = std::make_unique<MockTrain>(1, TrainLine::FOUR, ServiceType::EXPRESS, nullptr);
        mock_train_2 = std::make_unique<MockTrain>(2, TrainLine::FOUR, ServiceType::EXPRESS, nullptr);

        mock_yard_1 = std::make_unique<MockStation>(1, "yard 1", true, lines);
        mock_signal_yard_1 = std::make_unique<MockSignal>(1, 1);
        mock_platform_yard_1 = std::make_unique<MockPlatform>(1, 1, mock_signal_yard_1.get(), mock_yard_1.get(), Direction::DOWNTOWN);

        mock_signal_track_1 = std::make_unique<MockSignal>(2, 2);
        mock_track_1 = std::make_unique<MockTrack>(2, mock_signal_track_1.get());

        mock_station = std::make_unique<MockStation>(2, "station", false, lines);
        mock_signal_station = std::make_unique<MockSignal>(3, 3);
        mock_platform_station = std::make_unique<MockPlatform>(3, 1, mock_signal_station.get(), mock_station.get(), Direction::DOWNTOWN);

        mock_signal_track_2 = std::make_unique<MockSignal>(4, 4);
        mock_track_2 = std::make_unique<MockTrack>(4, mock_signal_track_2.get());

        mock_yard_2 = std::make_unique<MockStation>(3, "yard 2", true, lines);
        mock_signal_yard_2 = std::make_unique<MockSignal>(5, 5);
        mock_platform_yard_2 = std::make_unique<MockPlatform>(5, 1, mock_signal_yard_2.get(), mock_yard_2.get(), Direction::DOWNTOWN);

        mock_yard_1->add_platform(mock_platform_yard_1.get());
        mock_station->add_platform(mock_platform_station.get());
        mock_yard_2->add_platform(mock_platform_yard_2.get());
    }

    void build_network()
    {
        // build network
        // yard 1 platform --- track 1
        mock_platform_yard_1->set_next(mock_track_1.get());
        mock_track_1->set_prev(mock_platform_yard_1.get());

        // track 1 --- station platform
        mock_track_1->set_next(mock_platform_station.get());
        mock_platform_station->set_prev(mock_track_1.get());

        // station platform --- track 2
        mock_platform_station->set_next(mock_track_2.get());
        mock_track_2->set_prev(mock_platform_station.get());

        // track 2 -- yard 2 platform
        mock_track_2->set_next(mock_platform_yard_2.get());
        mock_platform_yard_2->set_prev(mock_track_2.get());
    }

    void SetUp() override
    {
        std::ofstream file(test_file, std::ios::trunc);
        file.close();
        /*
        constructs:
        1 train, 2 yards, 1 station, 3 platforms, 2 tracks, 5 signals

        arranges  network:
            Yard 1 (platform + signal)
                |
            Track 1 (signal)
                |
            Station (platform + signal)
                |
            Track 2 (signal)
                |
            Yard 2 (platform + signal)
        */

        build_objects();
        build_network();

        EXPECT_EQ(mock_platform_yard_1->get_next(), mock_track_1.get());
        EXPECT_EQ(mock_track_1->get_next(), mock_platform_station.get());
        EXPECT_EQ(mock_platform_station->get_next(), mock_track_2.get());
        EXPECT_EQ(mock_track_2->get_next(), mock_platform_yard_2.get());

        // construct dispatch
        std::vector<Train *> trains{mock_train_1.get()};
        std::vector<Station *> stations{mock_yard_1.get(), mock_station.get(), mock_yard_2.get()};
        std::vector<Platform *> platforms{mock_platform_yard_1.get(), mock_platform_station.get(), mock_platform_yard_2.get()};
        std::vector<Track *> tracks{mock_track_1.get(), mock_track_2.get()};
        std::vector<Signal *> signals{mock_signal_yard_1.get(), mock_signal_track_1.get(), mock_signal_station.get(), mock_signal_track_2.get(), mock_signal_yard_2.get()};

        dispatch = std::make_unique<Dispatch>(stations, trains, tracks, platforms, signals, logger);
    }
};

TEST_F(DispatchTest, ConstructorInitializesCorrectly)
{
    const auto &stations_map = dispatch->get_stations();
    EXPECT_EQ(stations_map.size(), 3);

    EXPECT_THAT(stations_map, ::testing::Contains(::testing::Pair(mock_yard_1->get_id(), mock_yard_1.get())));
    EXPECT_THAT(stations_map, ::testing::Contains(::testing::Pair(mock_station->get_id(), mock_station.get())));
    EXPECT_THAT(stations_map, ::testing::Contains(::testing::Pair(mock_yard_2->get_id(), mock_yard_2.get())));

    EXPECT_THAT(dispatch->get_trains(), ::testing::ElementsAre(mock_train_1.get()));
    EXPECT_THAT(dispatch->get_signals(), ::testing::UnorderedElementsAre(
                                             mock_signal_yard_1.get(),
                                             mock_signal_track_1.get(),
                                             mock_signal_station.get(),
                                             mock_signal_track_2.get(),
                                             mock_signal_yard_2.get()));

    EXPECT_THAT(dispatch->get_segments(), ::testing::UnorderedElementsAre(
                                              mock_track_1.get(),
                                              mock_track_2.get(),
                                              static_cast<Platform *>(mock_platform_yard_1.get()),
                                              static_cast<Platform *>(mock_platform_station.get()),
                                              static_cast<Platform *>(mock_platform_yard_2.get())));

    const auto schedule = dispatch->get_schedule();
    EXPECT_EQ(schedule.size(), stations_map.size());

    for (const auto &[station_id, queues] : schedule)
    {
        EXPECT_TRUE(stations_map.contains(station_id));
        EXPECT_TRUE(queues.arrivals.empty());
        EXPECT_TRUE(queues.departures.empty());
    }
}

TEST_F(DispatchTest, LoadsScheduleAndConstructsPriorityQueuesSucessfully)
{
    std::ofstream out(test_schedule);
    ASSERT_TRUE(out.is_open());

    // write valid data
    out << "train_id,station_id,station_name,direction,arrival_tick,departure_tick\n";
    out << "1,1,Yard 1,downtown,-1,2\n";
    out << "1,2,Station,downtown,3,5\n";
    out.close();

    dispatch->load_schedule(test_schedule);
    const auto &schedule = dispatch->get_schedule();

    EXPECT_TRUE(schedule.contains(1));
    EXPECT_TRUE(schedule.contains(2));
    EXPECT_EQ(schedule.at(1).arrivals.size(), 0);
    EXPECT_EQ(schedule.at(1).departures.size(), 1);
    EXPECT_EQ(schedule.at(2).arrivals.size(), 1);
    EXPECT_EQ(schedule.at(2).departures.size(), 1);

    std::remove(test_schedule.c_str());
}

TEST_F(DispatchTest, ReturnsEarlyFromLoadScheduleForEmptyFile)
{
    dispatch->load_schedule(test_schedule);
    const auto &schedule = dispatch->get_schedule();

    for (const auto &[station_id, queues] : schedule)
    {
        EXPECT_TRUE(queues.arrivals.empty());
        EXPECT_TRUE(queues.departures.empty());
    }
    std::remove(test_schedule.c_str());
}

TEST_F(DispatchTest, SkipsMalformedLinesInLoadSchedule)
{
    std::ofstream out(test_schedule);
    ASSERT_TRUE(out.is_open());

    // write invalid data
    out << "train_id,station_id,station_name,direction,arrival_tick,departure_tick\n";
    out << "1,1,Yard 1,downtown,-1,2\n";
    out << "1,2,Station,downtown\n"; // malformed, should not add events for station 2 to schedule
    out.close();

    dispatch->load_schedule(test_schedule);
    const auto &schedule = dispatch->get_schedule();

    EXPECT_TRUE(schedule.contains(1));
    EXPECT_EQ(schedule.at(1).arrivals.size(), 0);
    EXPECT_EQ(schedule.at(1).departures.size(), 1);

    EXPECT_TRUE(schedule.at(2).arrivals.empty());
    EXPECT_TRUE(schedule.at(2).departures.empty());

    std::remove(test_schedule.c_str());
}

TEST_F(DispatchTest, SkipsLinesWithInvalidInputInLoadSchedule)
{
    std::ofstream out(test_schedule);
    ASSERT_TRUE(out.is_open());

    // write invalid data
    out << "train_id,station_id,station_name,direction,arrival_tick,departure_tick\n";
    out << "1,1,Yard 1,downtown,-1,2\n";
    out << "1,2,Station,downtown,t,k\n"; // invalid number, should skip
    out << "1,3,Station,up,3,4\n";       // invalid direction, should skip
    out.close();

    dispatch->load_schedule(test_schedule);
    const auto &schedule = dispatch->get_schedule();

    EXPECT_TRUE(schedule.contains(1));
    EXPECT_EQ(schedule.at(1).arrivals.size(), 0);
    EXPECT_EQ(schedule.at(1).departures.size(), 1);

    EXPECT_TRUE(schedule.at(2).arrivals.empty());
    EXPECT_TRUE(schedule.at(2).departures.empty());
    EXPECT_TRUE(schedule.at(3).arrivals.empty());
    EXPECT_TRUE(schedule.at(3).departures.empty());

    std::remove(test_schedule.c_str());
}

TEST_F(DispatchTest, UpdateHandlesDelayedSignals)
{
    mock_train_1->spawn(mock_platform_yard_1.get());
    mock_signal_track_1->set_delay(2);

    dispatch->update(0);
    EXPECT_TRUE(mock_signal_track_1->is_red());
    EXPECT_EQ(mock_train_1->get_current_track(), static_cast<Track *>(mock_platform_yard_1.get()));
}

TEST_F(DispatchTest, UpdateMovesTrainIfAllowed)
{
    mock_train_1->spawn(mock_platform_yard_1.get());
    dispatch->update(0);

    EXPECT_EQ(mock_train_1->get_current_track(), mock_track_1.get());
}

TEST_F(DispatchTest, UpdateHandlesOccupiedSignals)
{
    mock_train_1->spawn(mock_platform_yard_1.get());
    dispatch->update(0);
    mock_signal_station->set_delay(2); // delay to force first train to stay

    mock_train_2->spawn(mock_platform_yard_1.get());
    dispatch->update(1);

    // in same place, because next track is occupied
    EXPECT_EQ(mock_train_2->get_current_track(), static_cast<Track *>(mock_platform_yard_1.get()));
    EXPECT_EQ(mock_train_1->get_current_track(), mock_track_1.get());

    EXPECT_TRUE(mock_track_1->is_occupied());
    EXPECT_TRUE(mock_signal_track_1->is_red());
}

TEST_F(DispatchTest, UpdateHandlesDespawnTrainOnYardArrival)
{
    mock_train_1->spawn(mock_platform_station.get());
    dispatch->update(0); // moves to track_2
    dispatch->update(1); // moves to yard_2

    EXPECT_EQ(mock_train_1->get_current_track(), nullptr);
    EXPECT_EQ(mock_train_1->get_status(), TrainStatus::OUTOFSERVICE);
}

TEST_F(DispatchTest, UpdateHandlesSpawnTrainsOnYardDeparture)
{
    EXPECT_EQ(mock_train_1->get_current_track(), nullptr);

    std::ofstream out(test_schedule);
    ASSERT_TRUE(out.is_open());

    // write valid data
    out << "train_id,station_id,station_name,direction,arrival_tick,departure_tick\n";
    out << "1,1,Yard 1,downtown,-1,0\n";
    out << "1,2,Station,downtown,3,5\n";
    out.close();

    dispatch->load_schedule(test_schedule);

    dispatch->update(0);

    EXPECT_EQ(mock_train_1->get_current_track(), static_cast<Track *>(mock_platform_yard_1.get()));
    std::remove(test_schedule.c_str());
}