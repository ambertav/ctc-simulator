#include <string>
#include <vector>
#include <filesystem>
#include <iostream>

#include "config.h"

#include "processor.h"
#include "subway_config.h"
#include "mnr_config.h"

int main()
{
    std::filesystem::create_directories(LOG_DIRECTORY);
    std::filesystem::create_directories(DATA_DIRECTORY);

    std::vector<etl::SystemConfig> systems{
        get_subway_config(),
        get_mnr_config(),
        // get_lirr_config()
    };

    for (const auto &system : systems)
    {
        auto station_out_path = std::filesystem::path(system.station_output_file);
        auto routes_out_path = std::filesystem::path(system.routes_output_file);

        std::filesystem::create_directories(station_out_path.parent_path());

        if (std::filesystem::exists(system.station_output_file))
        {
            std::cout << "[SKIPPING] cleaned station file already exists for " << system.name << "\n";
        }
        else
        {
            std::cout << "[PROCESSING] stations for " << system.name << "...\n";
            etl::process_stations(system);
        }

        if (std::filesystem::exists(system.routes_output_file))
        {
            std::cout << "[SKIPPING] cleaned routes file already exists for " << system.name << "\n";
        }
        else
        {
            std::cout << "[PROCESSING] routes for " << system.name << "...\n";
            etl::process_routes(system);
        }
    }

    return 0;
}