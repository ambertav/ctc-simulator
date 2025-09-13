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
        MockPlatform(int i, Signal *si, const Station *st, Direction dir, int dw, std::unordered_set<TrainLine> lines) : Platform(i, si, st, dir, dw, lines) {}
        MOCK_METHOD(const Direction &, get_direction, (), (const, override));
        MOCK_METHOD(bool, supports_train_line, (TrainLine line), (const, override));
    };

    Direction platform_direction{SUB::Direction::DOWNTOWN};
    TrainLine platform_line{SUB::TrainLine::A};

    Station station{1, "station", false, {SUB::TrainLine::A, SUB::TrainLine::C, SUB::TrainLine::E}};
    Station yard{2, "yard", true, {SUB::TrainLine::A, SUB::TrainLine::C, SUB::TrainLine::E}};
    MockPlatform mock_platform{1, nullptr, &station, platform_direction, 2, {platform_line}};
};

TEST_F(StationTest, ConstructorInitializesCorrectly)
{
    EXPECT_EQ(station.get_id(), 1);
    EXPECT_EQ(station.get_name(), "station");
    EXPECT_FALSE(station.is_yard());
    EXPECT_THAT(station.get_train_lines(), ::testing::UnorderedElementsAre(SUB::TrainLine::A, SUB::TrainLine::C, SUB::TrainLine::E));

    EXPECT_TRUE(yard.is_yard());
}

TEST_F(StationTest, AddsPlatformSuccessfully)
{
    station.add_platform(&mock_platform);
    EXPECT_THAT(station.get_platforms(), ::testing::ElementsAre(&mock_platform));
}

TEST_F(StationTest, SelectPlatformReturnsMatchingPlatformSuccessfully)
{
    station.add_platform(&mock_platform);
    EXPECT_CALL(mock_platform, get_direction()).WillRepeatedly(::testing::ReturnRef(platform_direction));
    EXPECT_CALL(mock_platform, supports_train_line(::testing::Eq(TrainLine{platform_line}))).WillRepeatedly(::testing::Return(true));

    auto selected{station.select_platform(platform_direction, platform_line)};

    ASSERT_TRUE(selected.has_value());
    ASSERT_EQ(selected.value(), &mock_platform);
}

TEST_F(StationTest, SelectPlatformFailsIfDirectionIsWrong)
{
    Direction wrong_direction{SUB::Direction::UPTOWN};
    station.add_platform(&mock_platform);
    EXPECT_CALL(mock_platform, get_direction()).WillRepeatedly(::testing::ReturnRef(wrong_direction));
    EXPECT_CALL(mock_platform, supports_train_line(::testing::_)).Times(0);

    auto selected{station.select_platform(platform_direction, platform_line)};

    ASSERT_FALSE(selected.has_value());
}

TEST_F(StationTest, SelectPlatformFailsIfTrainLineIsWrong)
{
    TrainLine wrong_line{SUB::TrainLine::SEVEN};

    station.add_platform(&mock_platform);
    EXPECT_CALL(mock_platform, get_direction()).WillRepeatedly(::testing::ReturnRef(platform_direction));
    EXPECT_CALL(mock_platform, supports_train_line(::testing::Eq(TrainLine{wrong_line}))).WillRepeatedly(::testing::Return(false));

    auto selected{station.select_platform(platform_direction, wrong_line)};

    ASSERT_FALSE(selected.has_value());
}