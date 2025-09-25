#pragma once

#include <fstream>
#include <string_view>

#include "core/signal.h"
#include "core/train.h"
#include "core/station.h"
#include "core/platform.h"

class Logger
{
private:
    std::ofstream outfile;

public:
    Logger(const std::string &file_path);
    ~Logger();

    void log_arrival(int actual_tick, int planned_tick, Train *train, Platform *arrived);
    void log_departure(int actual_tick, int planned_tick, Train *train, Platform *departing);
    void log_signal_change(int tick, Signal *signal);
    void log_train_spawn(int actual_tick, int planned_tick, Train *train, Direction dir);
    void log_train_despawn(int actual_tick, int planned_tick, Train *train, Direction dir);
    void log_warning(std::string_view message);

private:
    void log(std::string_view message);
};
