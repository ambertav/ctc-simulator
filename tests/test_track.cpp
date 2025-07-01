#include <gtest/gtest.h>
#include <gmock/gmock.h>

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

    class MockTrain : public Train
    {
    public:
        MockTrain(int i, TrainLine l, ServiceType t) : Train(i, l, t) {}
    };

    MockTrain mock_train{1, TrainLine::A, ServiceType::EXPRESS};
    MockSignal mock_signal{1, 1};
    Track track{1, &mock_signal};
};

TEST_F(TrackTest, ConstructorInitializesCorrectly)
{

    EXPECT_EQ(track.get_id(), 1);
    EXPECT_EQ(track.get_signal(), &mock_signal);
    EXPECT_EQ(track.get_occupying_train(), nullptr);
    EXPECT_FALSE(track.is_occupied());
    EXPECT_FALSE(track.is_platform());
    EXPECT_EQ(track.get_next(), nullptr);
    EXPECT_EQ(track.get_prev(), nullptr);
}

TEST_F(TrackTest, AllowsEntrySuccessfully)
{
    EXPECT_CALL(mock_signal, is_green()).WillOnce(testing::Return(true));
    EXPECT_TRUE(track.allow_entry());
}

TEST_F(TrackTest, DeniesEntryIfSignalIsNotGreen)
{
    EXPECT_CALL(mock_signal, is_green()).WillOnce(testing::Return(false));
    EXPECT_FALSE(track.allow_entry());
}

TEST_F(TrackTest, AcceptsEntrySuccessfully)
{
    track.accept_entry(&mock_train);
    EXPECT_TRUE(track.is_occupied());
    EXPECT_EQ(track.get_occupying_train(), &mock_train);
}

TEST_F(TrackTest, FailsEntryIfNullptr)
{
    track.accept_entry(nullptr);
    EXPECT_FALSE(track.is_occupied());
}

TEST_F(TrackTest, ReleasesTrainSuccessfully)
{
    track.accept_entry(&mock_train);
    track.release_train();
    EXPECT_FALSE(track.is_occupied());
    EXPECT_EQ(track.get_occupying_train(), nullptr);
}
