#include <vector>
#include <filesystem>
#include <optional>
#include <iostream>

#include "config.h"

#include "map/subway.h"
#include "map/metro_north.h"
#include "map/lirr.h"
#include "core/factory.h"
#include "core/dispatch.h"
#include "systems/logger.h"
#include "systems/scheduler.h"

int main()
{
    std::filesystem::create_directories(LOG_DIRECTORY);
    std::filesystem::create_directories(DATA_DIRECTORY);

    std::string schedule_file{std::string(DATA_DIRECTORY) + "/schedule.csv"};
    std::string logger_file{std::string(LOG_DIRECTORY) + "/sim.txt"};

    // Transit::Map::Subway &subway{Transit::Map::Subway::get_instance()};
    // std::optional<Transit::Map::Path> path_opt{subway.find_path(384, 610)};

    // Transit::Map::MetroNorth &mnr{Transit::Map::MetroNorth::get_instance()};
    // std::optional<Transit::Map::Path> path_opt{mnr.find_path(56, 114)};

    Transit::Map::LongIslandRailroad &lirr{Transit::Map::LongIslandRailroad::get_instance()};
    std::optional<Transit::Map::Path> path_opt{lirr.find_path(11, 237)};

    if (!path_opt.has_value())
    {
        throw std::runtime_error("No path found");
    }

    Transit::Map::Path path {*path_opt};

    Scheduler scheduler{schedule_file};
    scheduler.create_schedule(path);

    Factory factory;
    factory.build_network(6, path);
    auto train_ptrs{factory.get_trains()};
    auto station_ptrs{factory.get_stations()};
    auto signal_ptrs{factory.get_signals()};
    auto platform_ptrs{factory.get_platforms()};
    auto track_ptrs{factory.get_tracks()}; 

    Logger logger{logger_file};
    Dispatch dispatch{station_ptrs, train_ptrs, track_ptrs, platform_ptrs, signal_ptrs, logger};
    dispatch.load_schedule(schedule_file);

    std::cout << "\n\n";

    // run loop
    for (int i = 0; i <= 100; i++)
    {
        dispatch.update(i);
    }

    return 0;
}