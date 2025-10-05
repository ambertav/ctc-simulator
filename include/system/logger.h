#pragma once

#include <fstream>
#include <string_view>

#include "constants/constants.h"
#include "core/switch.h"
#include "core/signal.h"
#include "core/train.h"
#include "core/station.h"
#include "core/platform.h"

class CentralLogger;

class Logger
{
private:
    std::ofstream outfile;
    Constants::System system_code;

public:
    Logger(const std::string &file_path, Constants::System sc);
    ~Logger();

    void info(std::string_view message);
    void warn(std::string_view message);
    void critical(std::string_view message);
};
