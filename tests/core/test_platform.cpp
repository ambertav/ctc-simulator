#include <gtest/gtest.h>
#include <gmock/gmock.h>

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

    Platform platform{1, 1, &mock_signal, &mock_station, SUB::Direction::DOWNTOWN};
};

TEST_F(PlatformTest, ConstructorInitializesCorrectly)
{
    EXPECT_EQ(platform.get_id(), 1);
    EXPECT_EQ(platform.get_signal(), &mock_signal);
    EXPECT_EQ(platform.get_occupying_train(), nullptr);
    EXPECT_FALSE(platform.is_occupied());
    EXPECT_TRUE(platform.is_platform());
    EXPECT_EQ(platform.get_next(), nullptr);
    EXPECT_EQ(platform.get_prev(), nullptr);
    EXPECT_EQ(platform.get_station(), &mock_station);

    auto dir_ptr = std::get_if<SUB::Direction>(&platform.get_direction());
    ASSERT_NE(dir_ptr, nullptr);
    EXPECT_EQ(*dir_ptr, SUB::Direction::DOWNTOWN);
}

TEST_F(PlatformTest, AllowsEntrySuccessfully)
{
    EXPECT_CALL(mock_signal, is_green()).WillOnce(testing::Return(true));
    EXPECT_TRUE(platform.allow_entry());
}

TEST_F(PlatformTest, DeniesEntryIfSignalIsNotGreen)
{
    EXPECT_CALL(mock_signal, is_green()).WillOnce(testing::Return(false));
    EXPECT_FALSE(platform.allow_entry());
}

TEST_F(PlatformTest, AcceptsEntrySuccessfully)
{
    platform.accept_entry(&mock_train);
    EXPECT_TRUE(platform.is_occupied());
    EXPECT_EQ(platform.get_occupying_train(), &mock_train);
}

TEST_F(PlatformTest, FailsEntryIfNullptr)
{
    platform.accept_entry(nullptr);
    EXPECT_FALSE(platform.is_occupied());
}

TEST_F(PlatformTest, ReleasesTrainSuccessfully)
{
    platform.accept_entry(&mock_train);
    platform.release_train();
    EXPECT_FALSE(platform.is_occupied());
    EXPECT_EQ(platform.get_occupying_train(), nullptr);
}