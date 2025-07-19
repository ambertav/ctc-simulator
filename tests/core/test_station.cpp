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
        MOCK_METHOD(const Direction&, get_direction, (), (const, override));
    };

    Direction platform_direction {SUB::Direction::DOWNTOWN};

    Station station{1, "station", false, {SUB::TrainLine::A, SUB::TrainLine::C, SUB::TrainLine::E}};
    Station yard{2, "yard", true, {SUB::TrainLine::A, SUB::TrainLine::C, SUB::TrainLine::E}};
    MockPlatform mock_platform{1, 1, nullptr, &station, platform_direction};

};

TEST_F(StationTest, ConstructorInitializesCorrectly)
{
    EXPECT_EQ(station.get_id(), 1);
    EXPECT_EQ(station.get_name(), "station");
    EXPECT_FALSE(station.is_yard());
    EXPECT_THAT(station.get_lines(), ::testing::ElementsAre(SUB::TrainLine::A, SUB::TrainLine::C, SUB::TrainLine::E));

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
    EXPECT_CALL(mock_platform, get_direction()).WillOnce(testing::ReturnRef(platform_direction));
    EXPECT_CALL(mock_platform, allow_entry()).WillOnce(testing::Return(true));

    EXPECT_EQ(station.find_available_platform(platform_direction), &mock_platform);
}

TEST_F(StationTest, FailsToFindAvailablePlatformIfDenyingEntry)
{
    station.add_platform(&mock_platform);
    EXPECT_CALL(mock_platform, allow_entry()).WillOnce(testing::Return(false));
    EXPECT_CALL(mock_platform, get_direction()).WillOnce(testing::ReturnRef(platform_direction));

    EXPECT_EQ(station.find_available_platform(platform_direction), std::nullopt);
}

TEST_F(StationTest, GetsPlatformsByDirectionSuccessfully)
{
    station.add_platform(&mock_platform);
    EXPECT_CALL(mock_platform, get_direction()).WillOnce(testing::ReturnRef(platform_direction));

    EXPECT_THAT(station.get_platforms_by_direction(platform_direction), ::testing::ElementsAre(&mock_platform));
}

TEST_F(StationTest, ReturnsEmptyIfNoMatchingPlatformDirections)
{
    station.add_platform(&mock_platform);
    EXPECT_CALL(mock_platform, get_direction()).WillOnce(testing::ReturnRef(platform_direction));

    Direction opposite_direction { directions_equal(platform_direction, SUB::Direction::DOWNTOWN) ? SUB::Direction::UPTOWN : SUB::Direction::DOWNTOWN};
    EXPECT_THAT(station.get_platforms_by_direction(opposite_direction), ::testing::IsEmpty());
}