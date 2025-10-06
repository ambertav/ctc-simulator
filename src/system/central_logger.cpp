#include <filesystem>
#include <ranges>
#include <algorithm>

#include "config.h"
#include "system/central_logger.h"

CentralLogger::CentralLogger()
{
    std::string dir{std::string(LOG_DIRECTORY) + "/"};
    std::string file_path{dir + "main.json"};

    if (!std::filesystem::exists(dir))
    {
        std::filesystem::create_directories(dir);
    }

    outfile.open(file_path, std::ios::out | std::ios::trunc);
    if (!outfile.is_open())
    {
        throw std::runtime_error("Failed to open central log file: " + file_path);
    }

    output["agency"] = nlohmann::json::object();

    for (const auto &[system_name, system_code] : Constants::SYSTEMS)
    {
        output["agency"][system_name] = nlohmann::json::array();
    }
}

CentralLogger::~CentralLogger()
{
    if (outfile.is_open())
    {
        outfile.close();
    }
}

void CentralLogger::report(Constants::System system_code, std::string_view message)
{
    std::lock_guard<std::mutex> lock(mutex);

    auto it {std::ranges::find_if(Constants::SYSTEMS, [system_code](const auto &system)
    {
        return system.second == system_code;
    })};

    if (it == Constants::SYSTEMS.end())
    {
        return;
    }

    message_queue.emplace(it->first, std::string(message));
}

void CentralLogger::process()
{
    std::lock_guard<std::mutex> lock(mutex);

    while (!message_queue.empty())
    {
        Entry entry{message_queue.front()};
        message_queue.pop();

        output["agency"][entry.system].push_back(entry.message);

    }

    outfile.seekp(0);
    outfile << output.dump(2);
    outfile.flush();
}