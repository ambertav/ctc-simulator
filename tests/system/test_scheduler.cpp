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
    int system_code{static_cast<int>(Constants::System::METRO_NORTH)};
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

            const Transit::Map::Route *matching_route{nullptr};
            for (const auto &route : train_line_routes)
            {
                std::optional<Direction> direction{direction_from_string(train["direction"])};
                if (!direction.has_value())
                {
                    FAIL() << "direction from schedule file is not a valid direction";
                    continue;
                }

                if (directions_equal(route.direction, *direction))
                {
                    matching_route = &route;
                    break;
                }
            }

            if (matching_route == nullptr)
            {
                FAIL() << "no route found for line " << line_name << " with direction " << train["direction"];
                continue;
            }

            std::vector<int> expected_sequence{matching_route->sequence};
            EXPECT_EQ(actual_sequence.size(), expected_sequence.size()) << "actual and expected sequence should have the same length";

            for (int i{0}; i < std::min(actual_sequence.size(), expected_sequence.size()); ++i)
            {
                EXPECT_EQ(actual_sequence[i], expected_sequence[i]) 
                << "train " << train["train_id"] << " has station mismatch at position " << i << 
                "\n expected station id " << expected_sequence[i] << ", but got " << actual_sequence[i];
            }
        }
    }
}

TEST_F(SchedulerTest, OutputConsistency)
{
    std::string second_test_directory{test_directory + "_run2"};
    std::filesystem::create_directories(std::string(SCHED_DIRECTORY) + "/" + second_test_directory);

    scheduler.write_schedule(mnr, registry, test_directory, system_code);
    scheduler.write_schedule(mnr, registry, second_test_directory, system_code);

    std::string second_file_path{std::string(SCHED_DIRECTORY) + "/" + second_test_directory + "/schedule.json"};

    std::ifstream file_one(file_path);
    std::ifstream file_two(second_file_path);

    nlohmann::json schedule_one{};
    nlohmann::json schedule_two{};
    file_one >> schedule_one;
    file_two >> schedule_two;

    EXPECT_EQ(schedule_one, schedule_two) << "Consecutive runs on same system and graph should produce identical output";

    std::filesystem::remove_all(std::string(SCHED_DIRECTORY) + "/" + second_test_directory);
}