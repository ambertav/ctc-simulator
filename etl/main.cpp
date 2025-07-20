#include <string>
#include <vector>
#include <filesystem>
#include <iostream>

#include "processor.h"
#include "config.h"

int main()
{
    std::filesystem::create_directories(LOG_DIRECTORY);
    std::filesystem::create_directories(DATA_DIRECTORY);

    std::vector<etl::Job> jobs{
        {"subway",
         std::string(DATA_DIRECTORY) + "/raw/mta_subway_stations.csv",
         std::string(DATA_DIRECTORY) + "/raw/gtfs_subway/trips.txt",
         std::string(DATA_DIRECTORY) + "/raw/gtfs_subway/stop_times.txt",
         std::string(DATA_DIRECTORY) + "/clean/subway/stations.csv",
         std::string(DATA_DIRECTORY) + "/clean/subway/routes.csv"},

        {"mnr",
         std::string(DATA_DIRECTORY) + "/raw/gtfsmnr/stops.txt",
         std::string(DATA_DIRECTORY) + "/raw/gtfsmnr/trips.txt",
         std::string(DATA_DIRECTORY) + "/raw/gtfsmnr/stop_times.txt",
         std::string(DATA_DIRECTORY) + "/clean/mnr/stations.csv",
         std::string(DATA_DIRECTORY) + "/clean/mnr/routes.csv"},
    };

    for (const auto &job : jobs)
    {
        std::filesystem::create_directories(std::filesystem::path(job.output_station_file).parent_path());
        if (std::filesystem::exists(job.output_station_file))
        {
            std::cout << "[SKIPPING] cleaned station file already exists for " << job.name << "\n";
        }
        else
        {
            std::cout << "[PROCESSING] stations for " << job.name << "...\n";
            etl::process_stations(job);
        }

        if (std::filesystem::exists(job.output_routes_file))
        {
            std::cout << "[SKIPPING] cleaned routes file already exists for " << job.name << "\n";
        }
        else
        {
            std::cout << "[PROCESSING] routes for " << job.name << "...\n";
            etl::process_routes(job);
        }
    }

    return 0;
}