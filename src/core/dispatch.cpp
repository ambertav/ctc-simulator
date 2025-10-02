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

#include "utils/utils.h"
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

const std::vector<std::pair<Train *, Track *>> &Dispatch::get_authorizations() const
{
    return authorized;
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
            train->set_headsign(headsign);

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
    handle_spawns(tick);

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

            Switch *sw{next->get_inbound_switch()};

            if (sw != nullptr)
            {
                int priority{calculate_switch_priority(tick, train)};
                central_control->request_switch(train, sw, current, next, priority, tick, this);
                continue;
            }

            Signal *signal{next->get_signal()};
            auto signal_delay{randomize_delay(Constants::SIGNAL_FAILURE_PROBABILITY)};
            if (signal_delay.first == true)
            {
                int time_to_repair{signal_delay.second};
                signal->set_failure(time_to_repair);
                failed_signals.insert(signal);

                logger->log_signal_failure(tick, train, signal);
                continue;
            }

            handle_signal_state(tick, signal);
            authorized.emplace_back(train, next);
        }
    }
}

void Dispatch::execute(int tick)
{

    std::erase_if(failed_signals, [&](Signal *signal)
                  {
    signal->update_repair();
    if (signal->is_functional())
    {
        logger->log_signal_repair(tick, signal, train_line);
        return true;
    }
    else
    {
        return false;
    } });

    std::vector<std::pair<Train *, Track *>> switch_granted{central_control->get_granted_links(this)};

    for (const auto &[train, next_track] : switch_granted)
    {
        authorized.emplace_back(train, next_track);
        Signal *signal{next_track->get_signal()};
        handle_signal_state(tick, signal);
    }

    for (const auto &[train, next_track] : authorized)
    {
        bool moved{train->move_to_track(next_track)};
        Signal *signal{next_track->get_signal()};

        handle_signal_state(tick, signal, true);

        if (moved)
        {
            if (train->is_arriving())
            {
                Platform *arrival_platform{static_cast<Platform *>(train->get_current_track())};
                const Station *arrival_station{arrival_platform->get_station()};

                if (!arrival_station->is_yard())
                {
                    auto platform_delay{randomize_delay(Constants::PLATFORM_DELAY_PROBABILITY)};
                    if (platform_delay.first == true)
                    {
                        train->add_dwell(platform_delay.second);
                        logger->log_platform_delay(tick, train, arrival_platform);
                    }
                }

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

                if (departure_station->is_yard())
                {
                    continue;
                }

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
    }
}

void Dispatch::handle_signal_state(int tick, Signal *signal, bool set_red)
{
    Track *track{signal->get_track()};
    SignalState new_state{};

    if (set_red == true || track->is_occupied())
    {
        new_state = SignalState::RED;
    }
    else if (needs_yellow_signal(track))
    {
        new_state = SignalState::YELLOW;
    }
    else
    {
        new_state = SignalState::GREEN;
    }

    signal->change_state(new_state);
    logger->log_signal_change(tick, signal);
}

bool Dispatch::needs_yellow_signal(Track *track)
{
    if (track == nullptr)
    {
        return false;
    }

    Track *next{track->get_next_track(train_line)};
    if (next == nullptr)
    {
        return false;
    }

    Track *next_next{next->get_next_track(train_line)};
    return next->is_occupied() || (next_next && next_next->is_occupied());
}

std::optional<Event> Dispatch::process_event(int tick, std::multimap<int, Event> &event_map, Train *train)
{
    int train_id = train->get_id();

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

    handle_signal_state(tick, signal);

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
        logger->log_train_spawn(tick, event.tick, train);
        handle_signal_state(tick, signal);
    }
}

void Dispatch::despawn_train(int tick, const Event &event, Train *train, int yard_id)
{
    train->despawn();
    logger->log_train_despawn(tick, event.tick, train);
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

std::pair<bool, int> Dispatch::randomize_delay(double probability)
{
    bool is_delayed{Utils::coin_flip(probability)};
    int delay{0};

    if (is_delayed)
    {
        delay = std::max<int>(1, Utils::random_in_range(Constants::MAX_DELAY)) + 1;
    }

    return {is_delayed, delay};
}