#include <vector>
#include <filesystem>
#include <future>
#include <iostream>

#include "config.h"

#include "map/subway.h"
#include "map/metro_north.h"
#include "map/lirr.h"
#include "core/central_control.h"
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

                CentralControl cc {system_code, system_name, *graph, registry};
                for (int i{0}; i < 1000; ++i)
                {
                    cc.run(i);
                }
            } }));
    }

    for (auto &f : futures)
    {
        f.wait();
    }

    return 0;
}