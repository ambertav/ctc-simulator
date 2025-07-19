#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "core/platform.h"
#include "core/track.h"
#include "core/train.h"

class TrainTest : public testing::Test
{
protected:
    class MockTrack : public Track
    {
    public:
        MockTrack(int i, Signal *s, int d = 1) : Track(i, s, d) {}
        MOCK_METHOD(bool, allow_entry, (), (const, override));
    };
    class MockPlatform : public Platform {
        public :
            MockPlatform(int i, int dw, Signal *si, const Station *st, Direction dir) : Platform(i, dw, si, st, dir) {}
    };

    MockTrack mock_track_1{1, nullptr};
    MockTrack mock_track_2{2, nullptr};
    MockPlatform mock_platform{1, 1, nullptr, nullptr, SUB::Direction::DOWNTOWN};
    Train train{1, SUB::TrainLine::FOUR, ServiceType::EXPRESS, &mock_track_1};
};

TEST_F(TrainTest, ConstructorInitializesCorrectly)
{
    EXPECT_EQ(train.get_id(), 1);
    EXPECT_TRUE(trainlines_equal(train.get_line(), SUB::TrainLine::FOUR));
    EXPECT_EQ(train.get_type(), ServiceType::EXPRESS);
    EXPECT_EQ(train.get_current_track(), &mock_track_1);
    EXPECT_EQ(train.get_status(), TrainStatus::IDLE);
    EXPECT_EQ(train.get_destination(), nullptr);
}

TEST_F(TrainTest, AddsAdditionalDelaysSuccessfully) 
{
    int additional {2};
    train.add_delay(additional);
    EXPECT_EQ(train.get_delay(), additional);
}

TEST_F(TrainTest, RequestMovementSucceedsWhenTrainIsNotDelayed)
{
    EXPECT_EQ(train.get_delay(), 0);
    EXPECT_TRUE(train.request_movement());
}

TEST_F(TrainTest, RequestMovementDeductsFromDelayAndReturnsFalse)
{
    int delay {2};
    train.add_delay(delay);
    
    EXPECT_FALSE(train.request_movement());
    EXPECT_EQ(train.get_delay(), delay - 1);
}

TEST_F(TrainTest, MoveToTrackSucceedsWhenAllowed)
{
    mock_track_1.set_next(&mock_track_2);
    mock_track_2.set_prev(&mock_track_1);

    EXPECT_CALL(mock_track_2, allow_entry()).WillOnce(testing::Return(true));
    EXPECT_TRUE(train.move_to_track());
    EXPECT_EQ(train.get_current_track(), &mock_track_2);
    EXPECT_EQ(train.get_delay(), train.get_current_track()->get_duration());
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

TEST_F(TrainTest, SpawnsAtYardIsReady)
{
    train.spawn(&mock_platform);
    EXPECT_EQ(train.get_status(), TrainStatus::READY);
}

TEST_F(TrainTest, DespawnsToNullptrIsOutOfService)
{
    train.despawn();
    EXPECT_EQ(train.get_status(), TrainStatus::OUTOFSERVICE);
}