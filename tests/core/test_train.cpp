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
        MockTrack(int i, Signal *s, int d = 1, std::unordered_set<TrainLine> l = {}) : Track(i, s, d, l) {}
        MOCK_METHOD(bool, accept_entry, (Train *), (override));
    };
    class MockPlatform : public Platform
    {
    public:
        MockPlatform(int i, Signal *si, const Station *st, Direction dir, int dw = 2, std::unordered_set<TrainLine> l = {}) : Platform(i, si, st, dir, dw, l) {}
        MOCK_METHOD(bool, accept_entry, (Train *), (override));
    };

    std::unordered_set<TrainLine> train_lines{SUB::TrainLine::FOUR};

    MockTrack mock_track{1, nullptr, 1, train_lines};
    MockPlatform mock_platform{2, nullptr, nullptr, SUB::Direction::DOWNTOWN, 2, train_lines};
    Train train{1, *train_lines.begin(), ServiceType::EXPRESS, SUB::Direction::DOWNTOWN};
};

TEST_F(TrainTest, ConstructorInitializesCorrectly)
{
    EXPECT_EQ(train.get_id(), 1);
    EXPECT_EQ(train.get_dwell(), 0);
    EXPECT_EQ(train.get_lateness(), 0);
    EXPECT_TRUE(trainlines_equal(train.get_train_line(), SUB::TrainLine::FOUR));
    EXPECT_EQ(train.get_service_type(), ServiceType::EXPRESS);
    EXPECT_EQ(train.get_status(), TrainStatus::IDLE);
    EXPECT_EQ(std::get<SUB::Direction>(train.get_direction()), SUB::Direction::DOWNTOWN);
    EXPECT_EQ(train.get_current_track(), nullptr);
}

TEST_F(TrainTest, HandlesHeadsign)
{
    std::string headsign{"Train Headsign"};
    train.set_headsign(headsign);

    EXPECT_EQ(train.get_headsign(), headsign);
}

TEST_F(TrainTest, HandlesDwell)
{
    EXPECT_EQ(train.get_dwell(), 0);

    int invalid_dwell{-1};
    train.add_dwell(invalid_dwell);
    EXPECT_EQ(train.get_dwell(), 0) << "Dwell should not be negatve";

    int additional_dwell{2};
    train.add_dwell(additional_dwell);
    EXPECT_EQ(train.get_dwell(), additional_dwell);
}

TEST_F(TrainTest, HandlesPunctuality)
{
    int early{-2};
    train.set_lateness(early);
    EXPECT_EQ(train.get_lateness(), early);
    EXPECT_FALSE(train.is_late()) << "Train should not be late if negative punctuality delta";

    int late{5};
    train.set_lateness(late);
    EXPECT_TRUE(train.is_late()) << "Train should be late if positive punctuality delta";
}

TEST_F(TrainTest, RequestsMovementAndHandlesDwellTimer)
{
    int dwell{2};
    train.add_dwell(dwell);

    while (train.get_dwell() > 0)
    {
        EXPECT_FALSE(train.request_movement()) << "Train should not move when dwell timer is greater than 0";
    }

    EXPECT_TRUE(train.request_movement()) << "Train should move once dwell timer is 0";
}

TEST_F(TrainTest, MovesToTrackSuccessfully)
{
    EXPECT_CALL(mock_platform, accept_entry(&train)).WillOnce(::testing::Return(true));
    EXPECT_TRUE(train.move_to_track(&mock_platform));
    EXPECT_EQ(train.get_current_track(), &mock_platform);
    EXPECT_TRUE(train.is_arriving()) << "Train status should be ARRIVING after moving into Platform";

    EXPECT_CALL(mock_track, accept_entry(&train)).WillOnce(::testing::Return(true));
    EXPECT_TRUE(train.move_to_track(&mock_track));
    EXPECT_EQ(train.get_current_track(), &mock_track);
    EXPECT_TRUE(train.is_departing()) << "Train status should be DEPARTING after moving out from Platform";
}

TEST_F(TrainTest, MovesToTrackFailsIfTrackIsNullptr)
{
    EXPECT_FALSE(train.move_to_track(nullptr));
}

TEST_F(TrainTest, MovesToTrackFailsIfTrackCannotAcceptEntry)
{
    EXPECT_EQ(train.get_current_track(), nullptr);
    EXPECT_CALL(mock_track, accept_entry(&train)).WillOnce(::testing::Return(false));
    EXPECT_FALSE(train.move_to_track(&mock_track)) << "Train should not move to track that does not accept entry";
    EXPECT_EQ(train.get_current_track(), nullptr);
}

TEST_F(TrainTest, SpawnsAtYardIsReady)
{
    EXPECT_TRUE(train.is_idle());
    EXPECT_FALSE(train.is_active());

    EXPECT_CALL(mock_platform, accept_entry(&train)).WillOnce(::testing::Return(true));
    train.spawn(&mock_platform);
    EXPECT_TRUE(train.is_active());
}

TEST_F(TrainTest, DespawnsToNullptrIsOutOfService)
{
    EXPECT_CALL(mock_platform, accept_entry(&train)).WillOnce(::testing::Return(true));
    train.spawn(&mock_platform);

    train.despawn();
    EXPECT_FALSE(train.is_active());
}