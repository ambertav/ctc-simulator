#include <optional>
#include <fstream>

#include "config.h"
#include "constants.h"
#include "systems/scheduler.h"
#include "enums/transit_types.hpp"

Scheduler::Scheduler(const std::string &file_path, int stg, int dt, int nt) : outfile(file_path, std::ios::out | std::ios::trunc), spawn_tick_gap(stg), dwell_time(dt), number_of_trains(nt) {}

void Scheduler::create_schedule(const Transit::Map::Path &path)
{
    if (path.nodes.empty() || !path.nodes.front() || !path.nodes.back())
    {
        throw std::logic_error("Path is empty or has null nodes");
    }

    if (!outfile.is_open())
    {
        std::cerr << "Failed to open schedule.csv for writing.\n";
        return;
    }

    outfile << "train_id,station_id,station_name,direction,arrival_tick,departure_tick\n";

    static int train_id{1};

    auto write_schedule = [&](const std::vector<const Transit::Map::Node *> &stations, const std::vector<int> distances, Direction dir)
    {
        for (int i = 0; i < number_of_trains / 2; ++i)
        {
            int tick = i * spawn_tick_gap;
            
            auto [start_yard, end_yard] = Yards::get_yard_id_by_direction(dir);

            outfile << train_id << "," << start_yard << "," << Yards::get_yard_name(start_yard) << "," << dir << "," << -1 << "," << tick << "\n";

            for (int i = 0; i < stations.size(); ++i)
            {
                const Transit::Map::Node *node = stations[i];
                int arrival = ++tick;
                int departure = arrival + dwell_time;

                outfile << train_id << "," << node->id << "," << node->name << "," << dir << "," << arrival << "," << departure << "\n";

                tick = departure;

                if (i < stations.size() - 1)
                {
                    tick += distances[i];
                }
            }

            outfile << train_id << "," << end_yard << "," << Yards::get_yard_name(end_yard) << "," << dir << "," << tick + 1 << "," << "-1\n";

            ++train_id;
        }
    };

    const Transit::Map::Node* start { path.nodes.front() };
    const Transit::Map::Node* end { path.nodes.back() };

    static_assert(std::is_same_v<decltype(start), const Transit::Map::Node*>);
static_assert(std::is_same_v<decltype(end), const Transit::Map::Node*>);

    std::optional<Direction> o_opt = infer_direction(start->train_lines.front(), start->latitude, start->longitude, end->latitude, end->longitude);
    std::optional<Direction> r_opt = infer_direction(start->train_lines.front(), end->latitude, end->longitude, start->latitude, start->longitude);
    
    Direction original {*o_opt};
    Direction reverse {*r_opt};

    write_schedule(path.nodes, path.segment_weights, original);

    const std::vector<const Transit::Map::Node *> reversed_path(path.nodes.rbegin(), path.nodes.rend());
    const std::vector<int> reversed_distances(path.segment_weights.rbegin(), path.segment_weights.rend());

    write_schedule(reversed_path, reversed_distances, reverse);

    outfile.flush();
}