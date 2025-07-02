#include <fstream>
#include <sstream>
#include <string>
#include <ranges>
#include <iostream>

#include "core/dispatch.h"
#include "constants.h"

Dispatch::Dispatch(const std::vector<Station *> &st, const std::vector<Train *> &tn, const std::vector<Track *> &tk, const std::vector<Platform *> &p, const std::vector<Signal *> &si)
    : trains(tn), signals(si)
{
    segments.reserve(tk.size() + p.size());
    segments.insert(segments.end(), tk.begin(), tk.end());
    for (Platform *platform : p)
    {
        segments.push_back(static_cast<Track *>(platform));
    }

    for (Station *station : st)
    {
        schedule.emplace(station->get_id(), std::priority_queue<Event>{});
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

const std::unordered_map<int, std::priority_queue<Event>> &Dispatch::get_schedule() const
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

    int line_num;
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
        Direction dir;

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

        if (dir_str != "Downtown" && dir_str != "Uptown")
        {
            std::cerr << "Invalid direction input on line " << line_num << ": " << line << "\n";
            continue;
        }

        dir = dir_str == "Downtown" ? Direction::DOWNTOWN : Direction::UPTOWN;

        schedule[station_id].emplace(arrival_tick, train_id, station_id, dir, EventType::ARRIVAL);
        schedule[station_id].emplace(departure_tick, train_id, station_id, dir, EventType::DEPARTURE);
    }
}

void Dispatch::update(int tick)
{
    // handle_delays();
    handle_signals();

    for (Train *train : trains)
    {
        if (!train)
            continue;

        if (train->can_advance())
        {
            bool moved = train->move_to_track();
            if (train->get_status() == TrainStatus::ARRIVING)
            {

                Platform *arrived = static_cast<Platform *>(train->get_current_track());

                if (arrived->get_station()->is_yard())
                {
                    despawn_train(train, arrived->get_station()->get_id());
                }
                else
                {
                    Signal *next_signal = arrived->get_next()->get_signal();
                    if (next_signal)
                        next_signal->set_delay(2); // platform dwell
                }
            }
            if (train->get_status() == TrainStatus::DEPARTING)
            {
                // LOG ACTUAL VS PLANNED
            }
        }
        else
        {
            handle_spawns(tick);
            // LOG DEPART FROM YARD
        }
    }
}

void Dispatch::handle_signals()
{

    for (Track *segment : segments)
    {
        Signal *signal = segment->get_signal();
        if (!signal)
            continue;

        SignalState new_state;
        if (signal->is_delayed() || segment->is_occupied())
        {
            new_state = SignalState::RED;
        }
        else if (!signal->is_delayed() && !segment->is_occupied())
        {
            new_state = SignalState::GREEN;
        }

        bool changed = signal->change_state(new_state);
        if (changed)
        { // LOG CHANGE
        }

        if (signal->is_delayed())
            signal->decrement_delay();
    }
}

void Dispatch::handle_spawns(int tick)
{
    for (int id : Yards::ids)
    {
        auto &event_queue = schedule[id];

        while (!event_queue.empty() && event_queue.top().tick <= tick)
        {
            Event event = event_queue.top();
            event_queue.pop();

            if (event.type == EventType::DEPARTURE)
                spawn_train(event);
        }
    }
}

void Dispatch::spawn_train(Event event)
{
    Station *station = stations[event.station_id];

    auto platforms = station->get_platforms_by_direction(event.direction);
    if (platforms.empty())
    {
        std::cout << "No platforms found at yard " << event.station_id << " for direction " << event.direction << "\n";
        return;
    }

    Platform *platform = platforms[0];

    for (Train *train : trains)
    {
        if (event.train_id == train->get_id())
        {
            train->spawn(platform);
            break;
        }
    }
}

void Dispatch::despawn_train(Train *train, int yard_id)
{
    train->spawn(nullptr);
}
