#pragma once

#include <fstream>
#include <mutex>
#include <string>

#include "core/signal.h"
#include "core/train.h"
#include "core/station.h"
#include "core/platform.h"

class Logger
{
private:
    std::ofstream outfile;
    std::mutex mutex;

public:
    Logger(const std::string &file_path);
    ~Logger();

    void log_arrival(int actual_tick, int planned_tick, Train *train, Platform *arrived);
    void log_departure(int actual_tick, int planned_tick, Train *train, Platform *departing);
    void log_signal_change(int tick, Signal *signal);
    void log_train_spawn(int actual_tick, int planned_tick, Train *train, Direction dir);
    void log(const std::string &message);
};
