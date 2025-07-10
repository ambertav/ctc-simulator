#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <vector>
#include <string>
#include <unordered_set>
#include <fstream>
#include <thread>

#include "config.h"
#include "systems/logger.h"

class LoggerTest : public testing::Test
{
protected:
    class MockTrain : public Train
    {
    public:
        MockTrain(int i, TrainLine l, ServiceType t, Track *ct) : Train(i, l, t, ct) {}
    };
    class MockPlatform : public Platform
    {
    public:
        MockPlatform(int i, Signal *si, const Station *st, Direction d) : Platform(i, si, st, d) {}
    };
    class MockSignal : public Signal
    {
    public:
        MockSignal(int i, int t) : Signal(i, t) {}
    };
    class MockStation : public Station
    {
    public:
        MockStation(int i, const std::string &n, bool y, const std::vector<TrainLine> &l) : Station(i, n, y, l) {}
    };

    std::unique_ptr<MockTrain> mock_train;
    std::unique_ptr<MockStation> mock_station;
    std::unique_ptr<MockSignal> mock_signal;
    std::unique_ptr<MockPlatform> mock_platform;

    std::string valid_file{std::string(LOG_DIRECTORY) + "/test_logger.txt"};
    std::unique_ptr<Logger> logger;

    void SetUp() override
    {
        mock_train = std::make_unique<MockTrain>(1, TrainLine::FOUR, ServiceType::EXPRESS, nullptr);
        mock_station = std::make_unique<MockStation>(1, "station", false, std::vector<TrainLine>{TrainLine::FOUR});
        mock_signal = std::make_unique<MockSignal>(1, 1);
        mock_platform = std::make_unique<MockPlatform>(1, mock_signal.get(), mock_station.get(), Direction::DOWNTOWN);

        logger = std::make_unique<Logger>(valid_file);
    }

    void TearDown() override
    {
        std::remove(valid_file.c_str());
    }
};

TEST(LoggerTestNoFixture, ConstructorInitializesCorrectly)
{
    EXPECT_NO_THROW(Logger logger{std::string(LOG_DIRECTORY) + "/test_logger.txt"});
}

TEST(LoggerTestNoFixture, ConstructorThrowsErrorForInvalidPath)
{
    EXPECT_THROW(Logger logger{"/path/that/does/not/exist/log.txt"}, std::runtime_error);
}

TEST_F(LoggerTest, LogsSingleMessageToFileSuccessfully)
{
    std::string message{"This is the message"};
    logger->log(message);

    std::string expected_message = message + "\n";

    std::ifstream file(valid_file);
    ASSERT_TRUE(file.is_open());

    // std::getline will remove newline
    std::ostringstream oss;
    oss << file.rdbuf(); // reads full file
    std::string written_message = oss.str();
    file.close();

    EXPECT_EQ(expected_message, written_message);
}

TEST_F(LoggerTest, HandlesConcurrentLoggingWithoutRaceConditions)
{
    const int number_of_threads{10};
    const int logs_per_thread{50};

    auto log_function = [this, logs_per_thread](int thread_id)
    {
        for (int i = 0; i < logs_per_thread; i++)
        {
            logger->log("Thread " + std::to_string(thread_id) + " log #" + std::to_string(i));
        }
    };

    std::vector<std::thread> threads;

    for (int i = 0; i < number_of_threads; i++)
    {
        threads.emplace_back(log_function, i);
    }

    for (auto &t : threads)
    {
        t.join();
    }

    std::ifstream file(valid_file);
    ASSERT_TRUE(file.is_open());

    std::unordered_set<std::string> entries;
    std::string line;
    while (std::getline(file, line))
    {
        entries.insert(line);
    }
    file.close();

    EXPECT_EQ(entries.size(), number_of_threads * logs_per_thread);

    for (int i = 0; i < number_of_threads; i++)
    {
        for (int j = 0; j < logs_per_thread; j++)
        {
            std::string expected_entry = "Thread " + std::to_string(i) + " log #" + std::to_string(j);
            EXPECT_TRUE(entries.count(expected_entry) > 0) << "Missing log entry: " << expected_entry;
        }
    }
}