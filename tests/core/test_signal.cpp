#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <thread>

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
    EXPECT_EQ(signal.get_state(), SignalState::RED);

    EXPECT_TRUE(signal.is_red());
    EXPECT_FALSE(signal.is_yellow());
    EXPECT_FALSE(signal.is_green());
}

TEST_F(SignalTest, ThreadSafetyOfChangeState)
{
    signal.change_state(SignalState::RED);


    constexpr int number_of_threads{100};
    std::vector<std::thread> threads{};
    std::atomic<int> successful_changes{0};

    for (int i = 0; i < number_of_threads; ++i)
    {
        threads.emplace_back([&]{
            if (signal.change_state(SignalState::GREEN))
            {
                successful_changes.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    for (auto& t : threads)
    {
        t.join();
    }

    EXPECT_GE(successful_changes.load(), 1);
    EXPECT_TRUE(signal.is_green());
}

TEST_F(SignalTest, ThreadSafetyOfReadState)
{
    signal.change_state(SignalState::GREEN);

    constexpr int number_of_threads{100};
    std::vector<std::thread> threads{};
    std::atomic<int> successful_reads{0};

    for (int i = 0; i < number_of_threads; ++i)
    {
        threads.emplace_back([&]{
            if (signal.is_green())
            {
                successful_reads.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    for (auto& t : threads)
    {
        t.join();
    }

    EXPECT_EQ(successful_reads.load(), number_of_threads);
}