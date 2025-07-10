#include <fstream>

#include "config.h"
#include "constants.h"
#include "systems/scheduler.h"

void Scheduler::create_schedule(const Transit::Map::Path &path, const std::string &outfile) const
{
    if (path.path.empty() || !path.path.front() || !path.path.back())
    {
        throw std::logic_error("Path is empty or has null nodes");
    }

    std::ofstream file(outfile);
    if (!file.is_open())
    {
        std::cerr << "Failed to open schedule.csv for writing.\n";
        return;
    }

    file << "train_id,station_id,station_name,direction,arrival_tick,departure_tick\n";

    auto write_schedule = [&](const std::vector<const Transit::Map::Node *> &stations, const std::vector<int> distances, Direction dir)
    {
        for (int i = 0; i < number_of_trains; ++i)
        {
            int tick = i * spawn_tick_gap;

            int train_id, start_yard, end_yard;

            if (dir == Direction::UPTOWN)
            {
                train_id = i + 1;
                start_yard = Yards::ids[0];
                end_yard = Yards::ids[1];
            }
            else if (dir == Direction::DOWNTOWN)
            {
                train_id = i + 1 + number_of_trains;
                start_yard = Yards::ids[1];
                end_yard = Yards::ids[0];
            }

            file << train_id << "," << start_yard << "," << Yards::get_yard_name(start_yard) << "," << dir << "," << -1 << "," << tick << "\n";

            for (int i = 0; i < stations.size(); ++i)
            {
                const Transit::Map::Node *node = stations[i];
                int arrival = ++tick;
                int departure = arrival + dwell_time;

                file << train_id << "," << node->id << "," << node->name << "," << dir << "," << arrival << "," << departure << "\n";

                tick = departure;

                if (i < stations.size() - 1)
                {
                    tick += distances[i];
                }
            }

            file << train_id << "," << end_yard << "," << Yards::get_yard_name(end_yard) << "," << dir << "," << tick + 1 << "," << "-1\n";
        }
    };

    Direction forward = infer_direction(path.path.front(), path.path.back());
    Direction reverse = forward == Direction::UPTOWN ? Direction::DOWNTOWN : Direction::UPTOWN;

    write_schedule(path.path, path.segment_weights, forward);

    const std::vector<const Transit::Map::Node *> reversed_path(path.path.rbegin(), path.path.rend());
    const std::vector<int> revered_distances(path.segment_weights.rbegin(), path.segment_weights.rend());

    write_schedule(reversed_path, revered_distances, reverse);
}