#include <vector>
#include <filesystem>
#include <future>
#include <iostream>

#include "config.h"

#include "map/subway.h"
#include "map/metro_north.h"
#include "map/lirr.h"
#include "system/factory.h"
#include "core/dispatch.h"
#include "system/logger.h"
#include "system/scheduler.h"

int main()
{
    std::filesystem::create_directories(LOG_DIRECTORY);
    std::filesystem::create_directories(DATA_DIRECTORY);
    std::filesystem::create_directories(SCHED_DIRECTORY);

    Transit::Map::Subway &subway{Transit::Map::Subway::get_instance()};
    Transit::Map::MetroNorth &mnr{Transit::Map::MetroNorth::get_instance()};
    Transit::Map::LongIslandRailroad &lirr{Transit::Map::LongIslandRailroad::get_instance()};

    Registry &registry{Registry::get_instance()};
    Scheduler scheduler{};

    std::vector<std::future<void>> futures{};
    for (const auto &[system_name, system_code] : Constants::SYSTEMS)
    {
        futures.emplace_back(std::async(std::launch::async, [&, system_name, system_code]()
                                        {
            const Transit::Map::Graph *graph{nullptr};
            switch (system_code)
            {
            case Constants::System::SUBWAY:
            {
                graph = &subway;
                break;
            }
            case Constants::System::METRO_NORTH:
            {
                graph = &mnr;
                break;
            }
            case Constants::System::LIRR:
            {
                graph = &lirr;
                break;
            }
            }
            if (graph != nullptr)
            {
            scheduler.write_schedule(*graph, registry, system_name, static_cast<int>(system_code));
            } }));
    }

    for (auto &f : futures)
    {
        f.wait();
    }

    // Logger logger{logger_file};
    // Dispatch dispatch{station_ptrs, train_ptrs, track_ptrs, platform_ptrs, signal_ptrs, logger};
    // dispatch.load_schedule(schedule_file);

    // std::cout << "\n\n";

    // // run loop
    // for (int i = 0; i <= 100; i++)
    // {
    //     dispatch.update(i);
    // }

    return 0;
}