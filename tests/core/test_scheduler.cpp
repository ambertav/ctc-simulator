#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "config.h"
#include "core/scheduler.h"

class SchedulerTest : public ::testing::Test
{
protected:
    Scheduler scheduler;
    Transit::Map::Base graph;
    Transit::Map::Path valid_path;
    Transit::Map::Path invalid_path;
    std::string outfile{std::string(DATA_DIRECTORY) + "/test_schedule.csv"};

    int weight{2};

    void SetUp() override
    {
        std::ofstream file(outfile, std::ios::trunc);
        file.close();

        Transit::Map::Node *A = graph.add_node("A", "Station A", {TrainLine::FOUR}, {"1"});
        Transit::Map::Node *B = graph.add_node("B", "Station B", {TrainLine::FOUR}, {"2"});
        Transit::Map::Node *C = graph.add_node("C", "Station C", {TrainLine::FOUR}, {"3"});
        Transit::Map::Node *D = graph.add_node("D", "Station D", {TrainLine::FOUR}, {"4"});

        graph.add_edge(A, B, weight, {TrainLine::FOUR});
        graph.add_edge(B, C, weight, {TrainLine::FOUR});

        valid_path = graph.find_path("A", "C");
        invalid_path = graph.find_path("A", "D");
    }
};

TEST_F(SchedulerTest, ThrowsErrorWhenPathIsNull)
{
    EXPECT_THROW(scheduler.create_schedule(invalid_path, outfile), std::logic_error);
}

TEST_F(SchedulerTest, CreatesScheduleAndWritesToFileSuccessfully)
{
    scheduler.create_schedule(valid_path, outfile);

    int total_trains = 3 * 2;                        // number of trains (default 3) * 2
    int total_stations = valid_path.path.size() + 2; // path size + 2 yards

    int expected_line_count{1 + (total_trains * total_stations)}; // plus 1 for header

    std::ifstream file(outfile);
    ASSERT_TRUE(file.is_open());

    std::string line;
    int line_count{};
    std::getline(file, line);
    EXPECT_EQ(line, "train_id,station_id,station_name,direction,arrival_tick,departure_tick");
    ++line_count;

    while (std::getline(file, line))
    {
        ++line_count;
    }

    EXPECT_EQ(line_count, expected_line_count);
}
