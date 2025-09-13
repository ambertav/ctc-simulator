#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "core/switch.h"
#include "core/signal.h"
#include "core/train.h"
#include "core/track.h"

class TrackTest : public testing::Test
{
protected:
    class MockSignal : public Signal
    {
    public:
        MockSignal(int i, int t) : Signal(i, t) {}
        MOCK_METHOD(bool, is_green, (), (const, override));
    };

    class MockSwitch : public Switch
    {
    public:
        MockSwitch(int i) : Switch(i) {}
    };

    class MockTrain : public Train
    {
    public:
        MockTrain(int i, TrainLine l, ServiceType t, Direction d) : Train(i, l, t, d) {}
    };

    TrainLine train_line{SUB::TrainLine::A};
    MockTrain mock_train{1, train_line, ServiceType::EXPRESS, SUB::Direction::UPTOWN};
    MockSignal mock_signal{1, 1};
    Track track{1, &mock_signal, 5, {train_line}};
};

TEST_F(TrackTest, ConstructorInitializesCorrectly)
{
    EXPECT_EQ(track.get_id(), 1);
    EXPECT_EQ(track.get_signal(), &mock_signal);
    EXPECT_EQ(track.get_duration(), 5);
    EXPECT_EQ(track.get_occupying_train(), nullptr);
    EXPECT_FALSE(track.is_occupied());
    EXPECT_FALSE(track.is_platform());
    EXPECT_TRUE(track.supports_train_line(SUB::TrainLine::A));
}

TEST_F(TrackTest, AcceptsEntryOfTrainSuccessfully)
{
    EXPECT_CALL(mock_signal, is_green()).WillOnce(testing::Return(true));
    EXPECT_TRUE(track.accept_entry(&mock_train));
    EXPECT_TRUE(track.is_occupied());
    EXPECT_EQ(track.get_occupying_train(), &mock_train);
}

TEST_F(TrackTest, AcceptsEntryFailsWhenOccupied)
{
    EXPECT_CALL(mock_signal, is_green()).WillRepeatedly(testing::Return(true));
    EXPECT_TRUE(track.accept_entry(&mock_train));

    MockTrain mock_train_2{2, train_line, ServiceType::EXPRESS, SUB::Direction::UPTOWN};
    EXPECT_FALSE(track.accept_entry(&mock_train_2));

    EXPECT_EQ(track.get_occupying_train(), &mock_train);
}

TEST_F(TrackTest, AcceptEntryFailsWithNullptrTrain)
{
    EXPECT_FALSE(track.accept_entry(nullptr));
}

TEST_F(TrackTest, AcceptEntryFailsWhenRedSignal)
{
    EXPECT_CALL(mock_signal, is_green()).WillOnce(testing::Return(false));
    EXPECT_FALSE(track.accept_entry(&mock_train));
}

TEST_F(TrackTest, ReleasesTrainSuccessfully)
{
    EXPECT_CALL(mock_signal, is_green()).WillOnce(testing::Return(true));
    EXPECT_TRUE(track.accept_entry(&mock_train));

    track.release_train();
    EXPECT_FALSE(track.is_occupied());
    EXPECT_EQ(track.get_occupying_train(), nullptr);
}

TEST_F(TrackTest, AddsAndRemovesTrainLinesSuccessfully)
{
    TrainLine line{SUB::TrainLine::C};
    EXPECT_FALSE(track.supports_train_line(line));

    track.add_train_line(line);
    EXPECT_TRUE(track.supports_train_line(line));

    track.remove_train_line(line);
    EXPECT_FALSE(track.supports_train_line(line));
}

TEST_F(TrackTest, GetsTrainLineNextAndPrevTrackSuccessfully)
{
    Track track_in_line{2, nullptr, 2, {train_line}};
    Track track_out_line{3, nullptr, 2, {SUB::TrainLine::F}};

    track.add_next_track(&track_in_line);
    track.add_next_track(&track_out_line);
    track.add_prev_track(&track_in_line);
    track.add_prev_track(&track_out_line);

    Track *next{track.get_next_track(train_line)};
    Track *prev{track.get_prev_track(train_line)};

    EXPECT_EQ(next, &track_in_line);
    EXPECT_EQ(prev, &track_in_line);
}

TEST_F(TrackTest, ReturnsNullptrIfNoNextOrPrevTrackInTrainLine)
{
    Track track_out_line{2, nullptr, 2, {SUB::TrainLine::F}};

    track.add_next_track(&track_out_line);
    track.add_prev_track(&track_out_line);

    Track *next{track.get_next_track(train_line)};
    Track *prev{track.get_prev_track(train_line)};

    EXPECT_EQ(next, nullptr);
    EXPECT_EQ(prev, nullptr);
}

TEST_F(TrackTest, HandleNextTracks)
{
    Track next_track{2, nullptr};

    EXPECT_TRUE(track.get_next_tracks().empty());

    track.add_next_track(&next_track);
    auto next_tracks{track.get_next_tracks()};
    EXPECT_EQ(next_tracks.size(), 1);
    EXPECT_TRUE(std::ranges::find(next_tracks, &next_track) != next_tracks.end());

    track.remove_next_track(&next_track);
    EXPECT_TRUE(track.get_next_tracks().empty());
}

TEST_F(TrackTest, HandlePrevTracks)
{
    Track prev_track{2, nullptr};

    EXPECT_TRUE(track.get_prev_tracks().empty());

    track.add_prev_track(&prev_track);
    auto prev_tracks{track.get_prev_tracks()};
    EXPECT_EQ(prev_tracks.size(), 1);
    EXPECT_TRUE(std::ranges::find(prev_tracks, &prev_track) != prev_tracks.end());

    track.remove_prev_track(&prev_track);
    EXPECT_TRUE(track.get_prev_tracks().empty());
}

TEST_F(TrackTest, IgnoresAddingDuplicateTracks)
{
    Track tr{2, nullptr};

    track.add_next_track(&tr);
    track.add_next_track(&tr);
    EXPECT_EQ(track.get_next_tracks().size(), 1);

    track.add_prev_track(&tr);
    track.add_prev_track(&tr);
    EXPECT_EQ(track.get_prev_tracks().size(), 1);
}

TEST_F(TrackTest, HandleOutboundSwitches)
{
    MockSwitch outbound_sw{1};

    track.add_outbound_switch(&outbound_sw);
    EXPECT_EQ(track.get_outbound_switch(), &outbound_sw);

    track.remove_outbound_switch();
    EXPECT_EQ(track.get_outbound_switch(), nullptr);
}

TEST_F(TrackTest, HandleInboundSwitches)
{
    MockSwitch inbound_sw{1};

    track.add_inbound_switch(&inbound_sw);
    EXPECT_EQ(track.get_inbound_switch(), &inbound_sw);

    track.remove_inbound_switch();
    EXPECT_EQ(track.get_inbound_switch(), nullptr);
}