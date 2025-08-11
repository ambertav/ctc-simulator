#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <atomic>
#include <thread>

#include "utils/test_utils.h"
#include "core/switch.h"
#include "core/signal.h"
#include "core/train.h"
#include "core/track.h"

class TrackTest : public testing::Test
{
protected:
    class MockSignal : public Signal
    {
    public:
        MockSignal(int i, int t) : Signal(i, t) {}
        MOCK_METHOD(bool, is_green, (), (const, override));
    };

    class MockSwitch : public Switch
    {
    public:
        MockSwitch(int i) : Switch(i) {}
    };

    class MockTrain : public Train
    {
    public:
        MockTrain(int i, TrainLine l, ServiceType t) : Train(i, l, t, nullptr) {}
    };

    MockTrain mock_train{1, SUB::TrainLine::A, ServiceType::EXPRESS};
    MockSignal mock_signal{1, 1};
    Track track{1, &mock_signal, 5, {SUB::TrainLine::A}};
};

TEST_F(TrackTest, ConstructorInitializesCorrectly)
{
    EXPECT_EQ(track.get_id(), 1);
    EXPECT_EQ(track.get_signal(), &mock_signal);
    EXPECT_EQ(track.get_duration(), 5);
    EXPECT_EQ(track.get_occupying_train(), nullptr);
    EXPECT_FALSE(track.is_occupied());
    EXPECT_FALSE(track.is_platform());
    EXPECT_TRUE(track.supports_train_line(SUB::TrainLine::A));
}

TEST_F(TrackTest, TryEntryAcceptsTrainSuccessfully)
{
    EXPECT_CALL(mock_signal, is_green()).WillOnce(testing::Return(true));
    EXPECT_TRUE(track.try_entry(&mock_train));
    EXPECT_TRUE(track.is_occupied());
    EXPECT_EQ(track.get_occupying_train(), &mock_train);
}

TEST_F(TrackTest, TryEntryFailsWhenOccupied)
{
    EXPECT_CALL(mock_signal, is_green()).WillRepeatedly(testing::Return(true));
    EXPECT_TRUE(track.try_entry(&mock_train));

    MockTrain mock_train_2{2, SUB::TrainLine::A, ServiceType::EXPRESS};
    EXPECT_FALSE(track.try_entry(&mock_train_2));

    EXPECT_EQ(track.get_occupying_train(), &mock_train);
}

TEST_F(TrackTest, TryEntryFailsWithNullptrTrain)
{
    EXPECT_FALSE(track.try_entry(nullptr));
}

TEST_F(TrackTest, TryEntryFailsWithNullptrSignal)
{
    Track track_with_no_signal{2, nullptr};
    EXPECT_FALSE(track.try_entry(&mock_train));
}

TEST_F(TrackTest, TryEntryFailsWHenSignalRed)
{
    EXPECT_CALL(mock_signal, is_green()).WillOnce(testing::Return(false));
    EXPECT_FALSE(track.try_entry(&mock_train));
}

TEST_F(TrackTest, ThreadSafetyOfTryEntry)
{
    EXPECT_CALL(mock_signal, is_green()).WillRepeatedly(testing::Return(true));

    constexpr int number_of_threads{100};

    std::vector<std::thread> threads{};
    std::vector<MockTrain> trains{};

    threads.reserve(number_of_threads);
    trains.reserve(number_of_threads);

    std::atomic<int> atomic_success_index{-1};
    std::vector<bool> results(number_of_threads, false);

    for (int i = 0; i < number_of_threads; ++i)
    {
        trains.emplace_back(i, SUB::TrainLine::A, ServiceType::EXPRESS);
    }

    for (int i = 0; i < number_of_threads; ++i)
    {
        threads.emplace_back([&, i]()
                             {
            results[i] = track.try_entry(&trains[i]);

            if (results[i] == true)
            {
                atomic_success_index.store(i, std::memory_order_release);
            } });
    }

    for (auto &t : threads)
    {
        t.join();
    }

    auto successes{std::count(results.begin(), results.end(), true)};
    EXPECT_EQ(successes, 1);
    EXPECT_TRUE(track.is_occupied());

    int success_index{atomic_success_index.load(std::memory_order_acquire)};
    ASSERT_GE(success_index, 0);
    EXPECT_EQ(track.get_occupying_train(), &trains[success_index]);
}

TEST_F(TrackTest, ReleasesTrainSuccessfully)
{
    EXPECT_CALL(mock_signal, is_green()).WillOnce(testing::Return(true));
    EXPECT_TRUE(track.try_entry(&mock_train));

    track.release_train();
    EXPECT_FALSE(track.is_occupied());
    EXPECT_EQ(track.get_occupying_train(), nullptr);
}

TEST_F(TrackTest, AddsAndRemovesTrainLinesSuccessfully)
{
    TrainLine line{SUB::TrainLine::C};
    EXPECT_FALSE(track.supports_train_line(line));

    track.add_train_line(line);
    EXPECT_TRUE(track.supports_train_line(line));

    track.remove_train_line(line);
    EXPECT_FALSE(track.supports_train_line(line));
}

TEST_F(TrackTest, HandleNextTracks)
{
    Track next_track{2, nullptr};

    EXPECT_TRUE(track.get_next_tracks().empty());

    track.add_next_track(&next_track);
    auto next_tracks{track.get_next_tracks()};
    EXPECT_EQ(next_tracks.size(), 1);
    EXPECT_TRUE(std::ranges::find(next_tracks, &next_track) != next_tracks.end());

    track.remove_next_track(&next_track);
    EXPECT_TRUE(track.get_next_tracks().empty());
}

TEST_F(TrackTest, HandlePrevTracks)
{
    Track prev_track{2, nullptr};

    EXPECT_TRUE(track.get_prev_tracks().empty());

    track.add_prev_track(&prev_track);
    auto prev_tracks{track.get_prev_tracks()};
    EXPECT_EQ(prev_tracks.size(), 1);
    EXPECT_TRUE(std::ranges::find(prev_tracks, &prev_track) != prev_tracks.end());

    track.remove_prev_track(&prev_track);
    EXPECT_TRUE(track.get_prev_tracks().empty());
}

TEST_F(TrackTest, IgnoresAddingDuplicateTracks)
{
    Track tr{2, nullptr};

    track.add_next_track(&tr);
    track.add_next_track(&tr);
    EXPECT_EQ(track.get_next_tracks().size(), 1);

    track.add_prev_track(&tr);
    track.add_prev_track(&tr);
    EXPECT_EQ(track.get_prev_tracks().size(), 1);
}

TEST_F(TrackTest, ThreadSafetyOfWriteAndReadForTrackConnections)
{
    std::vector<std::unique_ptr<Track>> tracks{};

    for (int i = 0; i < 100; ++i)
    {
        tracks.emplace_back(std::make_unique<Track>(i, nullptr));
    }

    auto writer = [&](int i, std::atomic<bool> &done)
    {
        Track *t1{tracks[i * 2].get()};
        Track *t2{tracks[i * 2 + 1].get()};

        while (!done.load(std::memory_order_acquire))
        {
            track.add_next_track(t1);
            track.add_prev_track(t2);

            if (TestUtils::coin_flip())
            {
                track.remove_next_track(t1);
            }

            if (TestUtils::coin_flip())
            {
                track.remove_prev_track(t2);
            }
        }
    };

    auto reader = [&](int i, std::atomic<bool> &done)
    {
        while (!done.load(std::memory_order_acquire))
        {
            auto next_tracks{track.get_next_tracks()};
            for (auto *tr : next_tracks)
            {
                ASSERT_NE(tr, nullptr) << "Null track found in next_tracks";
                ASSERT_TRUE(std::find_if(tracks.begin(), tracks.end(), [&](const auto &ptr)
                                         { return ptr.get() == tr; }) != tracks.end())
                    << "Invalid track in next_tracks";
            }

            auto prev_tracks{track.get_prev_tracks()};
            for (auto *tr : prev_tracks)
            {
                ASSERT_NE(tr, nullptr) << "Null track found in prev_tracks";
                ASSERT_TRUE(std::find_if(tracks.begin(), tracks.end(), [&](const auto &ptr)
                                         { return ptr.get() == tr; }) != tracks.end())
                    << "Invalid track in prev_tracks";
            }
        }
    };

    TestUtils::run_concurrency_write_read(writer, reader);
}

TEST_F(TrackTest, HandleOutboundSwitches)
{
    MockSwitch outbound_sw{1};
    
    track.add_outbound_switch(&outbound_sw);
    EXPECT_EQ(track.get_outbound_switch(), &outbound_sw);

    track.remove_outbound_switch();
    EXPECT_EQ(track.get_outbound_switch(), nullptr);
}

TEST_F(TrackTest, HandleInboundSwitches)
{
    MockSwitch inbound_sw{1};

    track.add_inbound_switch(&inbound_sw);
    EXPECT_EQ(track.get_inbound_switch(), &inbound_sw);

    track.remove_inbound_switch();
    EXPECT_EQ(track.get_inbound_switch(), nullptr);
}