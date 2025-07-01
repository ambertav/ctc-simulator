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

    MockTrack mock_track_1{1, nullptr};
    MockTrack mock_track_2{2, nullptr};
    Train train{1, TrainLine::FOUR, ServiceType::EXPRESS, &mock_track_1};
};

TEST_F(TrainTest, ConstructorInitializesCorrectly)
{
    EXPECT_EQ(train.get_id(), 1);
    EXPECT_EQ(train.get_line(), TrainLine::FOUR);
    EXPECT_EQ(train.get_type(), ServiceType::EXPRESS);
    EXPECT_EQ(train.get_current_track(), &mock_track_1);
    EXPECT_EQ(train.get_status(), TrainStatus::IDLE);
    EXPECT_EQ(train.get_destination(), nullptr);
}

TEST_F(TrainTest, CanAdvanceWhenAllowed)
{
    mock_track_1.set_next(&mock_track_2);
    mock_track_2.set_prev(&mock_track_1);

    EXPECT_CALL(mock_track_2, allow_entry()).WillOnce(testing::Return(true));
    EXPECT_TRUE(train.can_advance());
}

TEST_F(TrainTest, CannotAdvanceIfNotAllowed)
{
    mock_track_1.set_next(&mock_track_2);
    mock_track_2.set_prev(&mock_track_1);

    EXPECT_CALL(mock_track_2, allow_entry()).WillOnce(testing::Return(false));
    EXPECT_FALSE(train.can_advance());
}

TEST_F(TrainTest, CannotAdvanceIfNullptr)
{
    mock_track_1.set_next(nullptr);
    mock_track_2.set_prev(nullptr);

    EXPECT_FALSE(train.can_advance());
}

TEST_F(TrainTest, MoveToTrackSucceedsWhenAllowed)
{
    mock_track_1.set_next(&mock_track_2);
    mock_track_2.set_prev(&mock_track_1);

    EXPECT_CALL(mock_track_2, allow_entry()).WillOnce(testing::Return(true));
    EXPECT_TRUE(train.move_to_track());
    EXPECT_EQ(train.get_current_track(), &mock_track_2);
}

TEST_F(TrainTest, MoveToTrackFailsWhenNotAllowed)
{
    mock_track_1.set_next(&mock_track_2);
    mock_track_2.set_prev(&mock_track_1);

    EXPECT_CALL(mock_track_2, allow_entry()).WillOnce(testing::Return(false));
    EXPECT_FALSE(train.move_to_track());
    EXPECT_EQ(train.get_current_track(), &mock_track_1);
}

TEST_F(TrainTest, MoveToTrackFailsWhenNullptr)
{
    mock_track_1.set_next(nullptr);
    mock_track_2.set_prev(nullptr);

    EXPECT_FALSE(train.move_to_track());
}