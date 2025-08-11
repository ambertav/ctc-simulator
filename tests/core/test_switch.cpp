#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <atomic>
#include <thread>

#include "utils/test_utils.h"
#include "core/switch.h"
#include "core/track.h"

class SwitchTest : public testing::Test
{
protected:
    Track track{1, nullptr};
    Switch sw{1};
};

TEST_F(SwitchTest, ConstructorInitializesCorrectly)
{
    EXPECT_EQ(sw.get_id(), 1);
    EXPECT_TRUE(sw.ready());
}

TEST_F(SwitchTest, HandlesAddingLinksCorrectly)
{
    Track input{1, nullptr};
    Track output{1, nullptr};

    sw.add_approach_track(&input);
    sw.add_departure_track(&output);

    EXPECT_TRUE(sw.set_link(&input, &output));
    EXPECT_EQ(sw.get_link(&input), &output);
}

TEST_F(SwitchTest, FailsToAddLinkForNonServicedTracks)
{
    Track input{1, nullptr};
    Track output{1, nullptr};

    EXPECT_FALSE(sw.set_link(&input, &output));
    EXPECT_EQ(sw.get_link(&input), nullptr);
}

TEST_F(SwitchTest, ThreadSafetyForWriteAndReadLinks)
{
    std::vector<std::unique_ptr<Track>> tracks{};

    for (int i = 0; i <= 100; ++i)
    {
        tracks.emplace_back(std::make_unique<Track>(i, nullptr));

        if (i % 2 == 0)
        {
            sw.add_approach_track(tracks.back().get());
        }
        else
        {
            sw.add_departure_track(tracks.back().get());
        }
    }

    auto writer = [&](int i, std::atomic<bool> &done)
    {
        Track *in{tracks[i * 2].get()};
        Track *out{tracks[i * 2 + 1].get()};

        while (!done.load(std::memory_order_acquire))
        {
            sw.set_link(in, out);
        }
    };

    auto reader = [&](int i, std::atomic<bool> &done)
    {
        while (!done.load(std::memory_order_acquire))
        {
            auto links{sw.get_links()};
            for (const auto &[in, out] : links)
            {
                ASSERT_NE(in, nullptr);
                ASSERT_NE(out, nullptr);
            }

            for (int i = 0; i < 100; ++i)
            {
                Track *in{tracks[i].get()};
                Track *linked{sw.get_link(in)};

                if (linked != nullptr)
                {
                    ASSERT_TRUE(std::find_if(tracks.begin(), tracks.end(), [&](const auto &ptr) {
                        return ptr.get() == linked; }) != tracks.end()) << "Invalid track in links";
                }
            }
        }
    };
}

TEST_F(SwitchTest, HandlesApproachTracks)
{
    EXPECT_TRUE(sw.get_approach_tracks().empty());

    sw.add_approach_track(&track);
    auto approaches{sw.get_approach_tracks()};
    EXPECT_EQ(approaches.size(), 1);
    EXPECT_TRUE(std::ranges::find(approaches, &track) != approaches.end());

    sw.remove_approach_track(&track);
    EXPECT_TRUE(sw.get_approach_tracks().empty());
}

TEST_F(SwitchTest, HandleDepartureTracks)
{
    EXPECT_TRUE(sw.get_departure_tracks().empty());

    sw.add_departure_track(&track);
    auto departures{sw.get_departure_tracks()};
    EXPECT_EQ(departures.size(), 1);
    EXPECT_TRUE(std::ranges::find(departures, &track) != departures.end());

    sw.remove_departure_track(&track);
    EXPECT_TRUE(sw.get_departure_tracks().empty());
}

TEST_F(SwitchTest, IgnoresAddingDuplicateTracks)
{
    sw.add_approach_track(&track);
    sw.add_approach_track(&track);
    EXPECT_EQ(sw.get_approach_tracks().size(), 1);

    sw.add_departure_track(&track);
    sw.add_departure_track(&track);
    EXPECT_EQ(sw.get_departure_tracks().size(), 1);
}

TEST_F(SwitchTest, ThreadSafetyOfWriteAndReadTrackConnections)
{
    std::vector<std::unique_ptr<Track>> tracks{};

    for (int i = 0; i < 100; ++i)
    {
        tracks.emplace_back(std::make_unique<Track>(i, nullptr));
    }

    auto writer = [&](int i, std::atomic<bool> &done)
    {
        Track *t1{tracks[i * 2].get()};
        Track *t2{tracks[i * 2 + 1].get()};

        while (!done.load(std::memory_order_acquire))
            ;
        {
            sw.add_approach_track(t1);
            sw.add_departure_track(t2);

            if (TestUtils::coin_flip())
            {
                sw.remove_approach_track(t1);
            }

            if (TestUtils::coin_flip())
            {
                sw.remove_departure_track(t2);
            }
        }
    };

    auto reader = [&](int i, std::atomic<bool> &done)
    {
        while (!done.load(std::memory_order_acquire))
        {
            auto approaches{sw.get_approach_tracks()};
            for (auto *tr : approaches)
            {
                ASSERT_NE(tr, nullptr) << "Null track found in approach_tracks";
                ASSERT_TRUE(std::find_if(tracks.begin(), tracks.end(), [&](const auto &ptr)
                                         { return ptr.get() == tr; }) != tracks.end())
                    << "Invalid track in approach_tracks";
            }

            auto departures{sw.get_departure_tracks()};
            for (auto *tr : departures)
            {
                ASSERT_NE(tr, nullptr) << "Null track found in departure_tracks";
                ASSERT_TRUE(std::find_if(tracks.begin(), tracks.end(), [&](const auto &ptr)
                                         { return ptr.get() == tr; }) != tracks.end())
                    << "Invalid track in departure_tracks";
            }
        }
    };

    TestUtils::run_concurrency_write_read(writer, reader);
}