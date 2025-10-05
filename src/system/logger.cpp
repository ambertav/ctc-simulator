#include <filesystem>
#include <fstream>
#include <stdexcept>

#include "system/logger.h"

Logger::Logger(const std::string &file_path, Constants::System sc) : system_code(sc)
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

void Logger::info(std::string_view message)
{
    outfile << "[INFO] " << message << "\n";
    outfile.flush();
}

void Logger::warn(std::string_view message)
{
    outfile << "[WARNING] " << message << "\n";
    outfile.flush();
}

void Logger::critical(std::string_view message)
{
    outfile << "[CRITICAL] " << message << "\n";
    outfile.flush();
    // central_logger.report(system_code, message);
}