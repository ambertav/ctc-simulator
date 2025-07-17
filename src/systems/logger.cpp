#include "systems/logger.h"

Logger::Logger(const std::string &file_path)
    : outfile(file_path, std::ios::out | std::ios::trunc)
{
    if (!outfile.is_open())
    {
        throw std::runtime_error("Failed to open log file: " + file_path);
    }
}

Logger::~Logger()
{
    if (outfile.is_open())
        outfile.close();
}

void Logger::log_arrival(int actual_tick, int planned_tick, Train *train, Platform *arrived)
{
    std::string message = "Train " + std::to_string(train->get_id()) +
                          " arrived at station " + arrived->get_station()->get_name() +
                          " on platform " + std::to_string(arrived->get_id()) +
                          " (actual tick: " + std::to_string(actual_tick) +
                          ", planned tick: " + std::to_string(planned_tick) + ")";

    log(message);
}

void Logger::log_departure(int actual_tick, int planned_tick, Train *train, Platform *departing)
{
    std::string message = "Train " + std::to_string(train->get_id()) +
                          " is departing station " + departing->get_station()->get_name() +
                          " from platform " + std::to_string(departing->get_id()) +
                          " (actual tick: " + std::to_string(actual_tick) +
                          ", planned tick: " + std::to_string(planned_tick) + ")";

    log(message);
}

void Logger::log_signal_change(int tick, Signal *signal)
{
    std::string state;
    if (signal->is_green())
    {
        state = "green";
    }
    else if (signal->is_yellow())
    {
        state = "yellow";
    }
    else if (signal->is_red())
    {
        state = "red";
    }

    std::string message = "Signal " + std::to_string(signal->get_id()) +
                          " changed state to " + state +
                          " at tick " + std::to_string(tick);
    log(message);
}

void Logger::log_train_spawn(int actual_tick, int planned_tick, Train *train, Direction dir)
{

    std::string message = "Train " + std::to_string(train->get_id()) +
                          " is leaving the yard" +
                          " (actual tick: " + std::to_string(actual_tick) +
                          ", planned tick: " + std::to_string(planned_tick) + ")";

    log(message);
}

void Logger::log(const std::string &message)
{
    std::lock_guard<std::mutex> lock(mutex);
    outfile << message << "\n";
    outfile.flush();
}
