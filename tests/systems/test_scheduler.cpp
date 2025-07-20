#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <optional>

#include "config.h"
#include "systems/scheduler.h"

class SchedulerTest : public ::testing::Test
{
protected:
    Scheduler scheduler{std::string(DATA_DIRECTORY) + "/test_schedule.csv"};
    Transit::Map::Graph graph;
    std::optional<Transit::Map::Path> valid_path_opt;
    std::optional<Transit::Map::Path> invalid_path_opt;
    std::string outfile{std::string(DATA_DIRECTORY) + "/test_schedule.csv"};

    std::unordered_map<char, int> name_to_id{
        {'A', 1},
        {'B', 2},
        {'C', 3},
        {'D', 4}};

    double weight{2.0};

    void SetUp() override
    {

        Transit::Map::Node *A = graph.add_node(name_to_id['A'], "Station A", {SUB::TrainLine::FOUR}, {"1"});
        Transit::Map::Node *B = graph.add_node(name_to_id['B'], "Station B", {SUB::TrainLine::FOUR}, {"2"});
        Transit::Map::Node *C = graph.add_node(name_to_id['C'], "Station C", {SUB::TrainLine::FOUR}, {"3"});
        Transit::Map::Node *D = graph.add_node(name_to_id['D'], "Station D", {SUB::TrainLine::FOUR}, {"4"});

        graph.add_edge(A, B, weight, {SUB::TrainLine::FOUR});
        graph.add_edge(B, C, weight, {SUB::TrainLine::FOUR});

        valid_path_opt = graph.find_path(name_to_id['A'],name_to_id['C']);
        invalid_path_opt = graph.find_path(name_to_id['A'], name_to_id['D']);
    }
};

TEST_F(SchedulerTest, ThrowsErrorWhenPathIsNull)
{
    ASSERT_FALSE(invalid_path_opt.has_value());
    EXPECT_THROW(scheduler.create_schedule(Transit::Map::Path{}), std::logic_error);
}

TEST_F(SchedulerTest, CreatesScheduleAndWritesToFileSuccessfully)
{
    ASSERT_TRUE(valid_path_opt.has_value());
    auto valid_path{*valid_path_opt};

    scheduler.create_schedule(valid_path);

    int total_trains = 3 * 2;                         // number of trains (default 3) * 2
    int total_stations = valid_path.nodes.size() + 2; // path size + 2 yards

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
