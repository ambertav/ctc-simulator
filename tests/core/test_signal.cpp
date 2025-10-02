#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "core/signal.h"

class SignalTest : public testing::Test
{
protected:
    Signal signal{1};
};

TEST_F(SignalTest, ConstructorInitializesCorrectly)
{
    EXPECT_EQ(signal.get_id(), 1);
    EXPECT_EQ(signal.get_state(), SignalState::RED);

    EXPECT_TRUE(signal.is_red());
    EXPECT_FALSE(signal.is_yellow());
    EXPECT_FALSE(signal.is_green());
}

TEST_F(SignalTest, HandlesStateChangesAndChecks)
{
    EXPECT_TRUE(signal.is_red());

    signal.change_state(SignalState::GREEN);
    EXPECT_TRUE(signal.is_green());

    signal.change_state(SignalState::YELLOW);
    EXPECT_TRUE(signal.is_yellow());

    SignalState new_state {SignalState::RED};
    signal.change_state(new_state);
    SignalState retrieved_state {signal.get_state()};
    EXPECT_EQ(retrieved_state, new_state);
}