#include <fstream>
#include <sstream>
#include <string>
#include <ranges>
#include <iostream>

#include "core/dispatch.h"
#include "constants.h"

Dispatch::Dispatch(const std::vector<Station *> &st, const std::vector<Train *> &tn, const std::vector<Track *> &tk, const std::vector<Platform *> &p, const std::vector<Signal *> &si, Logger &log)
    : trains(tn), signals(si), logger(log)
{
    segments.reserve(tk.size() + p.size());
    segments.insert(segments.end(), tk.begin(), tk.end());
    for (Platform *platform : p)
    {
        segments.push_back(static_cast<Track *>(platform));
    }

    for (Station *station : st)
    {
        schedule.emplace(station->get_id(), EventQueues{});
        stations[station->get_id()] = station;
    }
}

const std::unordered_map<int, Station *> &Dispatch::get_stations() const
{
    return stations;
}

const std::vector<Train *> &Dispatch::get_trains() const
{
    return trains;
}

const std::vector<Signal *> &Dispatch::get_signals() const
{
    return signals;
}

const std::vector<Track *> &Dispatch::get_segments() const
{
    return segments;
}

const std::unordered_map<int, EventQueues> &Dispatch::get_schedule() const
{
    return schedule;
}

void Dispatch::load_schedule(const std::string &csv_file)
{
    std::ifstream file(csv_file);
    if (!file.is_open())
    {
        std::cerr << "Failed to open schedule file\n";
        return;
    }

    std::string line;

    if (!std::getline(file, line))
    {
        std::cerr << "Schedule is empty or missing header: " << csv_file << "\n";
        return;
    }

    int line_num {};
    while (std::getline(file, line))
    {
        ++line_num;
        std::istringstream ss(line);
        std::string t_id_str, s_id_str, station_name, dir_str, a_tick_str, d_tick_str;

        if (!std::getline(ss, t_id_str, ',') ||
            !std::getline(ss, s_id_str, ',') ||
            !std::getline(ss, station_name, ',') ||
            !std::getline(ss, dir_str, ',') ||
            !std::getline(ss, a_tick_str, ',') ||
            !std::getline(ss, d_tick_str, ','))
        {
            std::cerr << "Malformed line " << line_num << ": " << line << "\n";
            continue;
        }

        int train_id, station_id, arrival_tick, departure_tick;

        try
        {
            train_id = std::stoi(t_id_str);
            station_id = std::stoi(s_id_str);
            arrival_tick = std::stoi(a_tick_str);
            departure_tick = std::stoi(d_tick_str);
        }
        catch (const std::invalid_argument &)
        {
            std::cerr << "Invalid format on line " << line_num << ": " << line << "\n";
            continue;
        }

        std::optional<Direction> dir_opt { direction_from_string(dir_str) };

        if (!dir_opt.has_value())
        {
            std::cerr << "Invalid direction input on line " << line_num << ": " << line << ", skipping...\n";
            continue;
        }

        Direction dir { *dir_opt };

        if (arrival_tick != -1)
        {
            schedule[station_id].arrivals.emplace(arrival_tick, train_id, station_id, dir, EventType::ARRIVAL);
        }

        if (departure_tick != -1)
        {
            schedule[station_id].departures.emplace(departure_tick, train_id, station_id, dir, EventType::DEPARTURE);
        }
    }
}

void Dispatch::update(int tick)
{
    // handle_delays();

    for (Train *train : trains)
    {
        if (!train || !train->is_active())
        {
            continue;
        }

        if (train->request_movement())
        {
            bool can_advance = authorize(tick, train);

            if (can_advance)
            {
                bool moved = train->move_to_track();

                if (train->get_status() == TrainStatus::ARRIVING)
                {
                    Platform *arrived = static_cast<Platform *>(train->get_current_track());

                    if (arrived->get_station()->is_yard())
                    {
                        despawn_train(train, arrived->get_station()->get_id());
                        continue;
                    }

                    auto &event_queue = schedule[arrived->get_station()->get_id()];

                    if (!event_queue.arrivals.empty())
                    {
                        Event event = event_queue.arrivals.top();

                        if (event.train_id == train->get_id())
                        {
                            event_queue.arrivals.pop();
                            logger.log_arrival(tick, event.tick, train, arrived);
                        }
                    }
                }
                else if (train->get_status() == TrainStatus::DEPARTING)
                {
                    Platform *departing = static_cast<Platform *>(train->get_current_track()->get_prev());

                    auto &event_queue = schedule[departing->get_station()->get_id()];

                    if (!event_queue.departures.empty())
                    {
                        Event event = event_queue.departures.top();

                        if (event.train_id == train->get_id())
                        {
                            event_queue.departures.pop();
                            logger.log_departure(tick, event.tick, train, departing);
                        }
                    }
                }
            }
        }
    }
    handle_spawns(tick);
}

bool Dispatch::authorize(int tick, Train *train)
{
    Track *next = train->get_current_track()->get_next();
    if (!next)
    {
        std::cerr << "No next segment available to authorize movement for train " << train->get_id() << "\n";
    }

    Signal *signal = next->get_signal();
    if (!signal)
    {
        std::cerr << "No signal available to authorize movement for train " << train->get_id() << "\n";
        return false;
    }

    SignalState new_state = SignalState::RED;
    if (!next->is_occupied())
    {
        new_state = SignalState::GREEN;
    }

    bool changed = signal->change_state(new_state);
    if (changed)
    {
        logger.log_signal_change(tick, signal);
    }

    return signal->is_green();
}

void Dispatch::handle_spawns(int tick)
{

    for (const auto &id : Yards::ids)
    {
        auto it = schedule.find(id);
        if (it == schedule.end())
        {
            continue;
        }

        auto &event_queue = it->second;

        if (!event_queue.departures.empty())
        {
            Event event = event_queue.departures.top();

            if (event.tick <= tick)
            {
                event_queue.departures.pop();
                spawn_train(tick, event);
            }
        }
    }
}

void Dispatch::spawn_train(int tick, Event event)
{
    Station *station = stations[event.station_id];

    auto platforms = station->get_platforms_by_direction(event.direction);
    if (platforms.empty())
    {
        std::cerr << "No platforms found at yard " << event.station_id << " for direction " << event.direction << "\n";
        return;
    }

    Platform *platform = platforms[0];

    for (Train *train : trains)
    {
        if (event.train_id == train->get_id() && train->get_status() == TrainStatus::IDLE)
        {
            train->spawn(platform);
            logger.log_train_spawn(tick, event.tick, train, event.direction);
        }
    }
}

void Dispatch::despawn_train(Train *train, int yard_id)
{
    train->despawn();
}
