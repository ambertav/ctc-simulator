#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "core/signal.h"

class SignalTest : public testing::Test
{
protected:
    Signal signal{1, 1};
};

TEST_F(SignalTest, ConstructorInitializesCorrectly)
{
    EXPECT_EQ(signal.get_id(), 1);
    EXPECT_EQ(signal.get_target(), 1);
    EXPECT_TRUE(signal.is_red());
    EXPECT_FALSE(signal.is_yellow());
    EXPECT_FALSE(signal.is_green());
}

TEST_F(SignalTest, SignalStateChangesCorrectlyAndReturnsIfChanged)
{
    EXPECT_TRUE(signal.change_state(SignalState::YELLOW));
    EXPECT_FALSE(signal.is_red());
    EXPECT_TRUE(signal.is_yellow());
    EXPECT_FALSE(signal.is_green());

    EXPECT_FALSE(signal.change_state(SignalState::YELLOW));

    EXPECT_TRUE(signal.change_state(SignalState::GREEN));
    EXPECT_FALSE(signal.is_red());
    EXPECT_FALSE(signal.is_yellow());
    EXPECT_TRUE(signal.is_green());

    EXPECT_FALSE(signal.change_state(SignalState::GREEN));

    EXPECT_TRUE(signal.change_state(SignalState::RED));
    EXPECT_TRUE(signal.is_red());
    EXPECT_FALSE(signal.is_yellow());
    EXPECT_FALSE(signal.is_green());

    EXPECT_FALSE(signal.change_state(SignalState::RED));
}