#pragma once

#include <fstream>
#include <string_view>

#include "constants/constants.h"
#include "core/switch.h"
#include "core/signal.h"
#include "core/train.h"
#include "core/station.h"
#include "core/platform.h"

class Logger
{
private:
    std::ofstream outfile;
    Constants::System system_code;

public:
    Logger(const std::string &file_path, Constants::System sc);
    ~Logger();

    void log_arrival(int actual_tick, int planned_tick, Train *train, Platform *arrived);
    void log_departure(int actual_tick, int planned_tick, Train *train, Platform *departing);
    void log_signal_change(int tick, Signal *signal);

    void log_train_spawn(int actual_tick, int planned_tick, Train *train);
    void log_train_despawn(int actual_tick, int planned_tick, Train *train);

    void log_platform_delay(int tick, Train *train, Platform *platform);
    void log_signal_failure(int tick, Train* train, Signal *signal);
    void log_switch_failure(int tick, Switch *sw);

    void log_signal_repair(int tick, Signal *signal, TrainLine train_line);
    void log_switch_repair(int tick, Switch *sw);

    void log_repair(std::string_view message);
    void log_failure(std::string_view message);
    void log_warning(std::string_view message);

private:
    void log(std::string_view message);
};
