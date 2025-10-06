#pragma once

#include <fstream>
#include <string_view>

#include "constants/constants.h"
#include "system/central_logger.h"

class Logger
{
private:
    std::ofstream outfile;
    Constants::System system_code;
    CentralLogger& central_logger;

public:
    Logger(const std::string &file_path, Constants::System sc, CentralLogger& cl);
    ~Logger();

    void info(std::string_view message);
    void warn(std::string_view message);
    void critical(std::string_view message);
};
