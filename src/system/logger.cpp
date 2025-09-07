#include <filesystem>
#include <fstream>
#include <stdexcept>

#include "system/logger.h"

Logger::Logger(const std::string &file_path)
{
    std::filesystem::path p(file_path);
    std::filesystem::path dir{p.parent_path()};

    if (!std::filesystem::exists(dir))
    {
        std::filesystem::create_directories(dir);
    }

    outfile.open(file_path, std::ios::out | std::ios::trunc);
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
    log("Train " + std::to_string(train->get_id()) +
        " arrived at station " + arrived->get_station()->get_name() +
        " on platform " + std::to_string(arrived->get_id()) +
        " (actual tick: " + std::to_string(actual_tick) +
        ", planned tick: " + std::to_string(planned_tick) + ")");
}

void Logger::log_departure(int actual_tick, int planned_tick, Train *train, Platform *departing)
{
    log("Train " + std::to_string(train->get_id()) +
        " is departing station " + departing->get_station()->get_name() +
        " from platform " + std::to_string(departing->get_id()) +
        " (actual tick: " + std::to_string(actual_tick) +
        ", planned tick: " + std::to_string(planned_tick) + ")");
}

void Logger::log_signal_change(int tick, Signal *signal)
{
    log("Signal " + std::to_string(signal->get_id()) +
        " changed state to " + signal_state_to_string(signal->get_state()) +
        " at tick " + std::to_string(tick));
}

void Logger::log_train_spawn(int actual_tick, int planned_tick, Train *train, Direction dir)
{

    log("Train " + std::to_string(train->get_id()) +
        " is leaving the yard" +
        " (actual tick: " + std::to_string(actual_tick) +
        ", planned tick: " + std::to_string(planned_tick) + ")");
}

void Logger::log_train_despawn(int actual_tick, int planned_tick, Train *train, Direction dir)
{
    log("Train " + std::to_string(train->get_id()) +
        " is arriving at yard" +
        " (actual tick: " + std::to_string(actual_tick) +
        ", planned tick: " + std::to_string(planned_tick) + ")");
}

void Logger::log_warning(std::string_view message)
{
    std::lock_guard<std::mutex> lock(mutex);
    outfile << "[WARNING] " << message << "\n";
}

void Logger::log(std::string_view message)
{
    std::lock_guard<std::mutex> lock(mutex);
    outfile << "[INFO] " << message << "\n";
}
