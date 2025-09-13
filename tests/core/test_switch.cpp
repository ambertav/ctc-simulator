#include <gtest/gtest.h>
#include <gmock/gmock.h>

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
}

TEST_F(SwitchTest, HandlesLinksSuccessfully)
{
    Track input_one{1, nullptr};
    Track input_two{2, nullptr};
    Track output_one{3, nullptr};
    Track output_two{4, nullptr};

    sw.add_approach_track(&input_one);
    sw.add_approach_track(&input_two);

    sw.add_departure_track(&output_one);
    sw.add_departure_track(&output_two);

    EXPECT_TRUE(sw.set_link(&input_one, &output_one));
    EXPECT_TRUE(sw.set_link(&input_two, &output_two));

    EXPECT_EQ(sw.get_link(&input_one), &output_one);
    EXPECT_THAT(sw.get_links(), ::testing::UnorderedElementsAre(
        ::testing::Pair(&input_one, &output_one),
        ::testing::Pair(&input_two, &output_two)
    ));
}

TEST_F(SwitchTest, FailsToAddLinkForNonServicedTracks)
{
    Track input{1, nullptr};
    Track output{1, nullptr};

    EXPECT_FALSE(sw.set_link(&input, &output));
    EXPECT_EQ(sw.get_link(&input), nullptr);
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