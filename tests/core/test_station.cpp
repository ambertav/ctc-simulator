#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "core/platform.h"
#include "core/station.h"

class StationTest : public testing::Test
{
protected:
    class MockPlatform : public Platform
    {
    public:
        MockPlatform(int i, int dw, Signal *si, const Station *st, Direction dir) : Platform(i, dw, si, st, dir) {}
        MOCK_METHOD(bool, allow_entry, (), (const, override));
        MOCK_METHOD(Direction, get_direction, (), (const, override));
    };

    Station station{1, "station", false, {TrainLine::A, TrainLine::C, TrainLine::E}};
    Station yard{2, "yard", true, {TrainLine::A, TrainLine::C, TrainLine::E}};
    MockPlatform mock_platform{1, 1, nullptr, &station, Direction::DOWNTOWN};
};

TEST_F(StationTest, ConstructorInitializesCorrectly)
{
    EXPECT_EQ(station.get_id(), 1);
    EXPECT_EQ(station.get_name(), "station");
    EXPECT_FALSE(station.is_yard());
    EXPECT_THAT(station.get_lines(), ::testing::ElementsAre(TrainLine::A, TrainLine::C, TrainLine::E));

    EXPECT_TRUE(yard.is_yard());
}

TEST_F(StationTest, AddsPlatformSuccessfully)
{
    station.add_platform(&mock_platform);
    EXPECT_THAT(station.get_platforms(), ::testing::ElementsAre(&mock_platform));
}

TEST_F(StationTest, FindsAvailablePlatformSuccessfully)
{
    station.add_platform(&mock_platform);
    EXPECT_CALL(mock_platform, get_direction()).WillOnce(testing::Return(Direction::DOWNTOWN));
    EXPECT_CALL(mock_platform, allow_entry()).WillOnce(testing::Return(true));

    EXPECT_EQ(station.find_available_platform(Direction::DOWNTOWN), &mock_platform);
}

TEST_F(StationTest, FailsToFindAvailablePlatformIfDenyingEntry)
{
    station.add_platform(&mock_platform);
    EXPECT_CALL(mock_platform, allow_entry()).WillOnce(testing::Return(false));
    EXPECT_CALL(mock_platform, get_direction()).WillOnce(testing::Return(Direction::DOWNTOWN));

    EXPECT_EQ(station.find_available_platform(Direction::DOWNTOWN), std::nullopt);
}

TEST_F(StationTest, GetsPlatformsByDirectionSuccessfully)
{
    station.add_platform(&mock_platform);
    EXPECT_CALL(mock_platform, get_direction()).WillOnce(testing::Return(Direction::DOWNTOWN));

    EXPECT_THAT(station.get_platforms_by_direction(Direction::DOWNTOWN), ::testing::ElementsAre(&mock_platform));
}

TEST_F(StationTest, ReturnsEmptyIfNoMatchingPlatformDirections)
{
    station.add_platform(&mock_platform);
    EXPECT_CALL(mock_platform, get_direction()).WillOnce(testing::Return(Direction::UPTOWN));

    EXPECT_THAT(station.get_platforms_by_direction(Direction::DOWNTOWN), ::testing::IsEmpty());
}