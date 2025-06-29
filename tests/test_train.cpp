#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "core/track.h"
#include "core/train.h"

class TrainTest : public testing::Test
{
protected:
    class MockTrack : public Track
    {
    public:
        MockTrack(int i, const Signal *s) : Track(i, s) {}
        MOCK_METHOD(bool, allow_entry, (), (const, override));
    };

    Train train{1, TrainLine::FOUR, ServiceType::EXPRESS};
    MockTrack mock_track{1, nullptr};
};

TEST_F(TrainTest, ConstructorInitializesCorrectly)
{
    EXPECT_EQ(train.get_id(), 1);
    EXPECT_EQ(train.get_line(), TrainLine::FOUR);
    EXPECT_EQ(train.get_type(), ServiceType::EXPRESS);
    EXPECT_EQ(train.get_current_track(), nullptr);
    EXPECT_EQ(train.get_status(), TrainStatus::IDLE);
    EXPECT_EQ(train.get_destination(), nullptr);
}

TEST_F(TrainTest, MoveToTrackSucceedsWhenAllowed)
{
    EXPECT_CALL(mock_track, allow_entry()).WillOnce(testing::Return(true));
    EXPECT_TRUE(train.move_to_track(&mock_track));
    EXPECT_EQ(train.get_current_track(), &mock_track);
}

TEST_F(TrainTest, MoveToTrackFailsWhenNotAllowed)
{
    EXPECT_CALL(mock_track, allow_entry()).WillOnce(testing::Return(false));
    EXPECT_FALSE(train.move_to_track(&mock_track));
}

TEST_F(TrainTest, MoveToTrackFailsWhenNullptr)
{
    EXPECT_FALSE(train.move_to_track(nullptr));
}