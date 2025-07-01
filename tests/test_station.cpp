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
        MockPlatform(int i, Signal *si, const Station *st, Direction d) : Platform(i, si, st, d) {}
        MOCK_METHOD(bool, allow_entry, (), (const, override));
        MOCK_METHOD(Direction, get_direction, (), (const, override));
    };

    Station station{1, "station", false, {TrainLine::A, TrainLine::C, TrainLine::E}};
    Station yard{2, "yard", true, {TrainLine::A, TrainLine::C, TrainLine::E}};
    std::unique_ptr<MockPlatform> mock_platform;

    void SetUp() override
    {
        mock_platform = std::make_unique<MockPlatform>(1, nullptr, &station, Direction::DOWNTOWN);
        ::testing::Mock::AllowLeak(mock_platform.get());
    }
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
    auto raw_ptr = mock_platform.get();

    station.add_platform(std::move(mock_platform));
    EXPECT_THAT(station.get_platforms(), ::testing::ElementsAre(raw_ptr));
}

TEST_F(StationTest, FindsAvailablePlatformSuccessfully)
{
    auto raw_ptr = mock_platform.get();

    station.add_platform(std::move(mock_platform));
    EXPECT_CALL(*raw_ptr, get_direction()).WillOnce(testing::Return(Direction::DOWNTOWN));
    EXPECT_CALL(*raw_ptr, allow_entry()).WillOnce(testing::Return(true));

    EXPECT_EQ(station.find_available_platform(Direction::DOWNTOWN), raw_ptr);

    std::cout << "mock_platform unique_ptr address: " << mock_platform.get() << "\n";
    std::cout << "raw_ptr address: " << raw_ptr << "\n";
}

TEST_F(StationTest, FailsToFindAvailablePlatformIfDenyingEntry)
{
    auto raw_ptr = mock_platform.get();

    station.add_platform(std::move(mock_platform));
    EXPECT_CALL(*raw_ptr, allow_entry()).WillOnce(testing::Return(false));
    EXPECT_CALL(*raw_ptr, get_direction()).WillOnce(testing::Return(Direction::DOWNTOWN));

    EXPECT_EQ(station.find_available_platform(Direction::DOWNTOWN), std::nullopt);
}

TEST_F(StationTest, GetsPlatformsByDirectionSuccessfully)
{
    auto raw_ptr = mock_platform.get();
    station.add_platform(std::move(mock_platform));
    EXPECT_CALL(*raw_ptr, get_direction()).WillOnce(testing::Return(Direction::DOWNTOWN));

    EXPECT_THAT(station.get_platforms_by_direction(Direction::DOWNTOWN), ::testing::ElementsAre((raw_ptr)));
}

TEST_F(StationTest, ReturnsEmptyIfNoMatchingPlatformDirections)
{
    auto raw_ptr = mock_platform.get();
    station.add_platform(std::move(mock_platform));
    EXPECT_CALL(*raw_ptr, get_direction()).WillOnce(testing::Return(Direction::UPTOWN));

    EXPECT_THAT(station.get_platforms_by_direction(Direction::DOWNTOWN), ::testing::IsEmpty());
}