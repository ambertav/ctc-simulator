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
        MockStation(int i, const std::string &n, bool y, const std::unordered_set<TrainLine> &l) : Station(i, n, y, l) {}
    };

    class MockTrain : public Train
    {
    public:
        MockTrain(int i, TrainLine l, ServiceType t, Direction d) : Train(i, l, t, d) {}
    };

    std::unordered_set<TrainLine> train_lines{SUB::TrainLine::SEVEN};

    MockTrain mock_train{1, *train_lines.begin(), ServiceType::LOCAL, SUB::Direction::DOWNTOWN};
    MockStation mock_station{1, "station", false, train_lines};
    MockSignal mock_signal{1, 1};

    Platform platform{1, &mock_signal, &mock_station, SUB::Direction::DOWNTOWN, 2, train_lines};
};

TEST_F(PlatformTest, ConstructorInitializesCorrectly)
{
    EXPECT_EQ(platform.get_id(), 1);
    EXPECT_EQ(platform.get_signal(), &mock_signal);
    EXPECT_EQ(platform.get_duration(), 2);
    EXPECT_EQ(platform.get_occupying_train(), nullptr);
    EXPECT_FALSE(platform.is_occupied());
    EXPECT_TRUE(platform.supports_train_line(*train_lines.begin()));
    EXPECT_EQ(platform.get_station(), &mock_station);
    EXPECT_EQ(std::get<SUB::Direction>(platform.get_direction()), SUB::Direction::DOWNTOWN);
    EXPECT_TRUE(platform.is_platform());
}