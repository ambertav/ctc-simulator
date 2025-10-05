#include <vector>
#include <filesystem>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <iostream>

#include "config.h"

#include "map/subway.h"
#include "map/metro_north.h"
#include "map/lirr.h"
#include "system/scheduler.h"
#include "core/agency_control.h"

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

    std::atomic<int> global_tick{0};
    std::mutex tick_mutex{};
    std::condition_variable tick_cv{};
    const int max_tick{500};

    int ready_threads{0};
    const int total_threads{Constants::SYSTEMS.size()};

    std::vector<std::thread> threads{};
    for (const auto &[system_name, system_code] : Constants::SYSTEMS)
    {
        threads.emplace_back([&, system_name, system_code]()
                             {
        try
        {
            const Transit::Map::Graph *graph{nullptr};
            switch (system_code)
            {
            case Constants::System::SUBWAY: graph = &subway; break;
            case Constants::System::METRO_NORTH: graph = &mnr; break;
            case Constants::System::LIRR: graph = &lirr; break;
            }

            if (graph != nullptr)
            {
                scheduler.write_schedule(*graph, registry, system_name, system_code);
                AgencyControl agency{system_code, system_name, *graph, registry};

                while (true)
                {
                    int tick {global_tick.load()};
                    if (tick >= max_tick)
                    {
                        break;
                    }

                    agency.run(tick);

                    {
                        std::unique_lock lock(tick_mutex);
                        ++ready_threads;
                        if (ready_threads == total_threads)
                        {
                            ready_threads = 0;
                            ++global_tick;
                            tick_cv.notify_all();
                        }
                        else
                        {
                            tick_cv.wait(lock, [&]{ return tick != global_tick.load(); });
                        }

                    }
                }
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "[ERROR] Simulation failed for system " << system_name << ": " << e.what() << "\n";
        } });
    }

    for (auto &t : threads)
    {
        t.join();
    }

    return 0;
}