#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <atomic>
#include <thread>

#include "core/train.h"
#include "core/signal.h"
#include "core/station.h"
#include "core/platform.h"

class PlatformTest : public testing::Test
{
protected:
    class MockSignal : public Signal
    {
    public:
        MockSignal(int i, int t) : Signal(i, t) {}
        MOCK_METHOD(bool, is_green, (), (const, override));
    };

    class MockStation : public Station
    {
    public:
        MockStation(int i, const std::string &n, bool y, const std::vector<TrainLine> &l) : Station(i, n, y, l) {}
    };

    class MockTrain : public Train
    {
    public:
        MockTrain(int i, TrainLine l, ServiceType t) : Train(i, l, t, nullptr) {}
    };

    MockTrain mock_train{1, SUB::TrainLine::SEVEN, ServiceType::LOCAL};
    MockStation mock_station{1, "station", false, {SUB::TrainLine::SEVEN}};
    MockSignal mock_signal{1, 1};

    Platform platform{1, &mock_signal, &mock_station, SUB::Direction::DOWNTOWN, 2, {SUB::TrainLine::SEVEN}};
};

TEST_F(PlatformTest, ConstructorInitializesCorrectly)
{
    EXPECT_EQ(platform.get_id(), 1);
    EXPECT_EQ(platform.get_signal(), &mock_signal);
    EXPECT_EQ(platform.get_duration(), 2);
    EXPECT_EQ(platform.get_occupying_train(), nullptr);
    EXPECT_FALSE(platform.is_occupied());
    EXPECT_TRUE(platform.is_platform());
    EXPECT_TRUE(platform.supports_train_line(SUB::TrainLine::SEVEN));
    EXPECT_EQ(platform.get_station(), &mock_station);

    auto dir_ptr = std::get_if<SUB::Direction>(&platform.get_direction());
    ASSERT_NE(dir_ptr, nullptr);
    EXPECT_EQ(*dir_ptr, SUB::Direction::DOWNTOWN);
}

TEST_F(PlatformTest, TryEntryAcceptsTrainSuccessfully)
{
    EXPECT_CALL(mock_signal, is_green()).WillOnce(testing::Return(true));
    EXPECT_TRUE(platform.try_entry(&mock_train));
    EXPECT_TRUE(platform.is_occupied());
    EXPECT_EQ(platform.get_occupying_train(), &mock_train);
}

TEST_F(PlatformTest, TryEntryThreadSafety)
{
    EXPECT_CALL(mock_signal, is_green()).WillRepeatedly(testing::Return(true));

    constexpr int number_of_threads{100};

    std::vector<std::thread> threads{};
    std::vector<MockTrain> trains{};

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
            results[i] = platform.try_entry(&trains[i]);
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
    EXPECT_TRUE(platform.is_occupied());

    int success_index{atomic_success_index.load(std::memory_order_acquire)};
    ASSERT_GE(success_index, 0);
    EXPECT_EQ(platform.get_occupying_train(), &trains[success_index]);
}

TEST_F(PlatformTest, ReleasesTrainSuccessfully)
{
    EXPECT_CALL(mock_signal, is_green()).WillOnce(testing::Return(true));
    EXPECT_TRUE(platform.try_entry(&mock_train));

    platform.release_train();
    EXPECT_FALSE(platform.is_occupied());
    EXPECT_EQ(platform.get_occupying_train(), nullptr);
}