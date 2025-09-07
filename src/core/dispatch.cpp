/**
 * for details on design, see:
 * docs/dispatch.md
 */

#include <fstream>
#include <sstream>
#include <string>
#include <ranges>
#include <algorithm>

#include <nlohmann/json.hpp>

#include "core/central_control.h"
#include "core/dispatch.h"
#include "constants/constants.h"

Dispatch::Dispatch(CentralControl *cc, TrainLine tl, const std::vector<Station *> &st, const std::vector<Train *> &tn, Logger *log)
    : central_control(cc), train_line(tl), trains(tn), logger(log)
{
    authorized.reserve(tn.size());

    stations.reserve(st.size());
    for (Station *station : st)
    {
        stations.emplace(station->get_id(), station);
        if (station->is_yard())
        {
            yards.push_back(station);
        }
        schedule.emplace(station->get_id(), EventQueues{});
    }

    schedule.reserve(st.size());
}

TrainLine Dispatch::get_train_line() const
{
    return train_line;
}

const std::unordered_map<int, Station *> &Dispatch::get_stations() const
{
    return stations;
}

const std::vector<Train *> &Dispatch::get_trains() const
{
    return trains;
}

const std::unordered_map<int, EventQueues> &Dispatch::get_station_schedules() const
{
    return schedule;
}

void Dispatch::load_schedule()
{
    using json = nlohmann::json;

    std::string file_path{std::string(SCHED_DIRECTORY) + "/" + central_control->get_system_name() + "/schedule.json"};

    std::ifstream file(file_path);
    if (!file.is_open())
    {
        std::cerr << "Failed to open schedule file for " << central_control->get_system_name() << " at " << train_line << " dispatch\n";
        return;
    }

    json input_json{};
    try
    {
        file >> input_json;
    }
    catch (const json::parse_error &e)
    {
        std::cerr << "Failed to parse JSON file " << file_path << ": " << e.what() << "\n";
        return;
    }

    std::string train_line_name{trainline_to_string(train_line)};

    if (!input_json.contains("train_lines") || !input_json["train_lines"].contains(train_line_name))
    {
        std::cerr << "Train line " << train_line_name << " not found in schedule\n";
    }

    auto trains_json{input_json["train_lines"][train_line_name]["trains"][0]};
    for (const auto &t_json : trains_json)
    {
        int train_id{t_json["train_id"]};
        std::string headsign{t_json["headsign"]};
        std::optional<Direction> dir_opt{direction_from_string(t_json["direction"])};
        if (!dir_opt.has_value())
        {
            std::cerr << "Invalid direction for train " << train_id << "\n";
            continue;
        }
        Direction direction{*dir_opt};

        for (const auto &s_json : t_json["schedule"])
        {
            auto it{std::ranges::find_if(trains, [train_id](Train *t)
                                         { return t->get_id() == train_id; })};
            if (it == trains.end())
            {
                continue;
            }

            Train *train{*it};
            train->add_headsign(headsign);

            int station_id{s_json["station_id"]};
            int arrival_tick{s_json["arrival_tick"]};
            int departure_tick{s_json["departure_tick"]};

            if (arrival_tick != -1)
            {
                schedule[station_id].arrivals.emplace(
                    arrival_tick,
                    Event{arrival_tick, train_id, station_id, direction, EventType::ARRIVAL});
            }

            if (departure_tick != -1)
            {
                schedule[station_id].departures.emplace(
                    departure_tick,
                    Event{departure_tick, train_id, station_id, direction, EventType::DEPARTURE});
            }
        }
    }
}

void Dispatch::authorize(int tick)
{
    authorized.clear();

    for (Train *train : trains)
    {
        if (train == nullptr || !train->is_active())
        {
            continue;
        }

        if (train->request_movement())
        {
            Track *current{train->get_current_track()};
            Track *next{current->get_next_track(train_line)};

            if (!next)
            {
                std::cerr << "No next segment available to authorize movement for train " << train->get_id() << "\n";
                continue;
            }

            if (next->is_occupied())
            {
                continue;
            }

            Switch *sw{current->get_outbound_switch()};

            if (sw != nullptr)
            {
                int priority{calculate_switch_priority(tick, train)};
                central_control->request_switch(train, sw, current, next, priority, tick, this);
                continue;
            }

            Signal *signal{next->get_signal()};
            signal->change_state(SignalState::GREEN);
            logger->log_signal_change(tick, signal);

            authorized.emplace_back(train, next);
        }
    }
}

void Dispatch::execute(int tick)
{
    std::vector<std::pair<Train *, Track *>> switch_granted{central_control->get_granted_links(this)};

    for (const auto &[train, next_track] : switch_granted)
    {
        authorized.emplace_back(train, next_track);
        Signal *signal{next_track->get_signal()};
        signal->change_state(SignalState::GREEN);
        logger->log_signal_change(tick, signal);
    }

    for (const auto &[train, next_track] : authorized)
    {
        bool moved{train->move_to_track(next_track)};
        Signal *signal{next_track->get_signal()};

        if (moved)
        {
            signal->change_state(SignalState::RED);

            if (train->is_arriving())
            {
                Platform *arrival_platform{static_cast<Platform *>(train->get_current_track())};
                const Station *arrival_station{arrival_platform->get_station()};

                auto &arrivals{schedule[arrival_station->get_id()].arrivals};
                std::optional<Event> event_opt{process_event(tick, arrivals, train)};

                if (event_opt.has_value())
                {
                    Event event{*event_opt};
                    train->set_lateness(tick - event.tick);
                    logger->log_arrival(tick, event.tick, train, arrival_platform);
                    if (arrival_station->is_yard())
                    {
                        despawn_train(tick, event, train, arrival_station->get_id());
                    }
                }
                else
                {
                    logger->log_warning("Non-scheduled arrival for train " + std::to_string(train->get_id()) + " at station " + arrival_station->get_name() + " (tick): " + std::to_string(tick));
                }
            }
            else if (train->is_departing())
            {
                Platform *departure_platform{static_cast<Platform *>(train->get_current_track()->get_prev_track(train_line))};
                const Station *departure_station{departure_platform->get_station()};

                auto &departures{schedule[departure_station->get_id()].departures};
                std::optional<Event> event_opt{process_event(tick, departures, train)};

                if (event_opt.has_value())
                {
                    Event event{*event_opt};
                    train->set_lateness(tick - event.tick);
                    logger->log_departure(tick, event.tick, train, departure_platform);
                }
                else
                {
                    logger->log_warning("Non-scheduled departure for train " + std::to_string(train->get_id()) + " at station " + departure_station->get_name() + " (tick): " + std::to_string(tick));
                }
            }
        }
        else
        {
            signal->change_state(SignalState::RED);
            // logger->log_signal_change(tick, signal);
        }
    }

    handle_spawns(tick);
}

// std::optional<Event> Dispatch::process_event(int tick, std::priority_queue<Event> &queue, Train *train)
// {
//     std::vector<Event> temp{};
//     int search_depth{0};

//     while (!queue.empty() && search_depth < Constants::MAX_EVENT_SEARCH_DEPTH)
//     {
//         Event event{queue.top()};

//         queue.pop();
//         ++search_depth;

//         if (tick - event.tick > Constants::EVENT_TIMEOUT)
//         {
//             logger->log_warning("Removing stale event for train " + std::to_string(event.train_id) + " with type " + event_type_to_string(event.type) + " at station " + std::to_string(event.station_id));
//             continue;
//         }

//         if (event.train_id == train->get_id())
//         {
//             for (const auto &temp_event : temp)
//             {
//                 queue.push(temp_event);
//             }
//             return event;
//         }
//         else
//         {
//             temp.push_back(event);
//         }
//     }

//     return std::nullopt;
// }

std::optional<Event> Dispatch::process_event(int tick, std::multimap<int, Event> &event_map, Train *train)
{
    int train_id = train->get_id();
    
    // Search all events for this train (no time limit)
    for (auto it = event_map.begin(); it != event_map.end(); ++it)
    {
        if (it->second.train_id == train_id)
        {
            Event found_event = it->second;
            event_map.erase(it);
            return found_event;
        }
    }
    
    return std::nullopt;
}

void Dispatch::handle_spawns(int tick)
{
    for (Station *yard : yards)
    {
        auto &departures{schedule[yard->get_id()].departures};
        
        auto it = departures.begin();
        while (it != departures.end() && it->first <= tick)
        {
            Event event = it->second;
            spawn_train(tick, event);
            it = departures.erase(it);
        }
    }
}

// void Dispatch::handle_spawns(int tick)
// {
//     for (Station *yard : yards)
//     {
//         auto &departures{schedule[yard->get_id()].departures};

//         while (!departures.empty() && departures.top().tick <= tick)
//         {
//             Event event{departures.top()};
//             departures.pop();
//             spawn_train(tick, event);
//         }
//     }
// }

void Dispatch::spawn_train(int tick, const Event &event)
{
    Station *station = stations[event.station_id];

    std::optional<Platform *> platform_opt{station->select_platform(event.direction, train_line)};
    if (!platform_opt.has_value())
    {
        std::cerr << "No platform found at yard " << event.station_id << " for direction " << event.direction << "\n";
        return;
    }

    Platform *platform{*platform_opt};
    Signal *signal{platform->get_signal()};
    signal->change_state(SignalState::GREEN);
    logger->log_signal_change(tick, signal);

    auto it{std::ranges::find_if(trains, [event](Train *t)
                                 { return t->get_id() == event.train_id; })};
    if (it == trains.end())
    {
        std::cerr << "No matching train found for event train id: " << event.train_id << " in spawn train\n";
        return;
    }

    Train *train{*it};

    if (train->is_idle())
    {
        train->spawn(platform);
        logger->log_train_spawn(tick, event.tick, train, event.direction);

        signal->change_state(SignalState::RED);
        logger->log_signal_change(tick, signal);
    }
}

void Dispatch::despawn_train(int tick, const Event &event, Train *train, int yard_id)
{
    train->despawn();
    logger->log_train_despawn(tick, event.tick, train, event.direction);
}

int Dispatch::calculate_switch_priority(int tick, Train *train)
{
    int priority{};

    if (train->is_late())
    {
        priority += train->get_lateness();
    }

    return priority;
}