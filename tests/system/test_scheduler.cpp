#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <nlohmann/json.hpp>

#include <fstream>
#include <filesystem>

#include "config.h"
#include "constants/constants.h"
#include "map/metro_north.h"
#include "system/scheduler.h"

class SchedulerTest : public ::testing::Test
{
protected:
    std::string test_directory{};
    std::string file_path{};

    Transit::Map::MetroNorth &mnr{Transit::Map::MetroNorth::get_instance()};
    Registry &registry{Registry::get_instance()};
    Constants::System system_code{Constants::System::METRO_NORTH};
    Scheduler scheduler{};

    void SetUp() override
    {
        test_directory = "test";
        std::filesystem::create_directories(std::string(SCHED_DIRECTORY) + "/" + test_directory);
        file_path = std::string(SCHED_DIRECTORY) + "/" + test_directory + "/schedule.json";
    }

    void TearDown() override
    {
        std::filesystem::remove_all(std::string(SCHED_DIRECTORY) + "/" + test_directory);
    }
};

TEST_F(SchedulerTest, CreatesScheduleSuccessfully)
{
    ASSERT_NO_THROW(scheduler.write_schedule(mnr, registry, test_directory, system_code));
    ASSERT_TRUE(std::filesystem::exists(file_path)) << "Schedule file should be created at: " << file_path;

    std::ifstream file(file_path);
    ASSERT_TRUE(file.is_open()) << "Should be able to open the schedule file";

    nlohmann::json json{};
    ASSERT_NO_THROW(file >> json) << "JSON should be valid";
}

TEST_F(SchedulerTest, ValidateScheduleDetails)
{
    scheduler.write_schedule(mnr, registry, test_directory, system_code);

    std::ifstream file(file_path);
    ASSERT_TRUE(file.is_open()) << "could not open file: " << file_path;
    nlohmann::json json{};
    file >> json;

    auto validate_train_schedule = [&](const nlohmann::json &train)
    {
        ASSERT_TRUE(train.contains("train_id"));
        ASSERT_TRUE(train.contains("direction"));
        ASSERT_TRUE(train.contains("headsign"));
        ASSERT_TRUE(train.contains("schedule"));

        int last_valid_time{-1};
        for (const auto &stop : train["schedule"])
        {
            ASSERT_TRUE(stop.contains("station_id"));
            ASSERT_TRUE(stop.contains("station_name"));
            ASSERT_TRUE(stop.contains("arrival_tick"));
            ASSERT_TRUE(stop.contains("departure_tick"));

            int arrival{stop["arrival_tick"].get<int>()};
            int departure{stop["departure_tick"].get<int>()};

            if (arrival != -1 && departure != -1)
            {
                EXPECT_LT(arrival, departure) << "arrival should be less then departure for station: " << stop["station_name"];
            }

            int current_time{(departure != -1) ? departure : arrival};
            if (current_time != -1 && last_valid_time != -1)
            {
                EXPECT_GT(current_time, last_valid_time) << "time should progress foward";
            }

            if (current_time != -1)
            {
                last_valid_time = current_time;
            }
        }
    };

    for (const auto &[line_name, line_data] : json["train_lines"].items())
    {
        ASSERT_TRUE(line_data.contains("trains")) << "train line " << line_name << " should contain trains";
        for (const auto &train : line_data["trains"])
        {
            validate_train_schedule(train);
        }
    }
}

TEST_F(SchedulerTest, ValidateRouteSequences)
{
    scheduler.write_schedule(mnr, registry, test_directory, system_code);
    const auto &routes{mnr.get_routes()};

    std::ifstream file(file_path);
    nlohmann::json json{};
    file >> json;

    for (const auto &[line_name, line_data] : json["train_lines"].items())
    {
        auto routes_it{routes.find(trainline_from_string(line_name))};
        if (routes_it == routes.end())
        {
            FAIL() << "train line " << line_name << " not found in routes";
            continue;
        }

        const auto &train_line_routes{routes_it->second};

        for (const auto &train : line_data["trains"])
        {
            std::vector<int> actual_sequence{};
            for (const auto &stop : train["schedule"])
            {
                std::string station_name{stop["station_name"]};
                if (station_name.find("yard") == std::string::npos)
                {
                    actual_sequence.push_back(stop["station_id"]);
                }
            }

            EXPECT_GT(actual_sequence.size(), 0) << "train " << train["train_id"] << " shoould have at least 1 non-yard station in route";

            std::vector<std::vector<int>> valid_sequences{};
            for (const auto &route : train_line_routes)
            {
                std::optional<Direction> direction{direction_from_string(train["direction"])};
                if (direction.has_value() && directions_equal(route.direction, *direction))
                {
                    valid_sequences.push_back(route.sequence);
                }
            }

            ASSERT_FALSE(valid_sequences.empty()) << "no valid routes found for line " << line_name << " with direction " << train["direction"];

            bool matches_any{std::any_of(valid_sequences.begin(), valid_sequences.end(), [&](const auto &sequence)
                                         {
                    if (seq.empty())
                    {
                        return false;
                    }
                    return actual_sequence.front() == seq.front() && actual_sequence.back() == seq.back(); })};

            EXPECT_TRUE(matches_any) << "train " << train["train_id"] << " had invalid start/end: " << actual_sequence.front() << " -> " << actual_sequence.back();
        }
    }
}