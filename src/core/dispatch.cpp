/**
 * for details on design, see:
 * docs/dispatch.md
 */

#include <fstream>
#include <sstream>
#include <string>
#include <ranges>
#include <algorithm>
#include <format>

#include <nlohmann/json.hpp>

#include "utils/utils.h"
#include "core/agency_control.h"
#include "core/dispatch.h"
#include "constants/constants.h"

Dispatch::Dispatch(AgencyControl *ac, TrainLine tl, const std::vector<Station *> &st, const std::vector<Train *> &tn, Logger *log)
    : agency_control(ac), train_line(tl), trains(tn), logger(log)
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

    std::string file_path{std::string(SCHED_DIRECTORY) + "/" + agency_control->get_system_name() + "/schedule.json"};

    std::ifstream file(file_path);
    if (!file.is_open())
    {
        std::cerr << "Failed to open schedule file for " << agency_control->get_system_name() << " at " << train_line << " dispatch\n";
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

    auto trains_json = input_json["train_lines"][train_line_name]["trains"];
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
                agency_control->request_switch(train, sw, current, next, priority, tick, this);
                continue;
            }

            Signal *signal{next->get_signal()};
            auto signal_delay{randomize_delay(Constants::SIGNAL_FAILURE_PROBABILITY)};
            if (signal_delay.first == true)
            {
                int time_to_repair{signal_delay.second};
                signal->set_failure(time_to_repair);
                failed_signals.insert(signal);

                logger->critical(std::format("Signal {} failed on {} line at tick {}", signal->get_id(), trainline_to_string(train_line), tick));
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
        logger->critical(std::format("Signal {} restored on {} line at tick {}", signal->get_id(), trainline_to_string(train_line), tick));
        return true;
    }
    else
    {
        return false;
    } });

    std::vector<std::pair<Train *, Track *>> switch_granted{agency_control->get_granted_links(this)};

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

                auto &arrivals{schedule[arrival_station->get_id()].arrivals};
                std::optional<Event> event_opt{process_event(tick, arrivals, train)};

                if (event_opt.has_value())
                {
                    Event event{*event_opt};
                    train->set_lateness(tick - event.tick);

                    logger->info(std::format("Train {} arrived at station {} on platform {} (actual tick {}, planned tick {})",
                                             train->get_id(),
                                             arrival_station->get_name(),
                                             arrival_platform->get_id(),
                                             tick,
                                             event.tick));

                    if (arrival_station->is_yard())
                    {
                        despawn_train(tick, event, train, arrival_station);
                    }
                }
                else
                {
                    logger->warn(std::format("Non-scheduled arrival for train {} at station {}, (tick {})", train->get_id(), arrival_station->get_name(), tick));
                }

                if (!arrival_station->is_yard())
                {
                    auto platform_delay{randomize_delay(Constants::PLATFORM_DELAY_PROBABILITY)};
                    if (platform_delay.first == true)
                    {
                        train->add_dwell(platform_delay.second);
                        logger->warn(std::format("Train {} is delayed at {} on platform {} at tick {}",
                                                 train->get_id(),
                                                 arrival_station->get_name(),
                                                 arrival_platform->get_id(),
                                                 tick));
                    }
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

                    logger->info(std::format("Train {} is departing station {} (actual tick {}, planned tick {})",
                                             train->get_id(),
                                             departure_station->get_name(),
                                             tick,
                                             event.tick));
                }
                else
                {
                    logger->warn(std::format("Non-scheduled departure for train {} at station {}, (tick {})", train->get_id(), departure_station->get_name(), tick));
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
    logger->info(std::format("Signal {} changed state to {} at tick {}", signal->get_id(), signal_state_to_string(signal->get_state()), tick));
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

        auto it{departures.begin()};
        while (it != departures.end() && it->first <= tick)
        {
            Event event = it->second;
            bool spawned{spawn_train(tick, event)};

            if (spawned)
            {
                it = departures.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
}

bool Dispatch::spawn_train(int tick, const Event &event)
{
    Station *station{stations[event.station_id]};

    std::vector<Platform *> platforms{station->select_platforms(event.direction, train_line)};
    if (platforms.empty())
    {
        std::cerr << "No platform found at yard " << event.station_id << " for direction " << event.direction << "\n";
        return false;
    }

    auto platform_it{std::ranges::find_if(platforms, [](Platform *p)
                                          { return !p->is_occupied(); })};
    if (platform_it == platforms.end())
    {
        return false;
    }
    Platform *platform{*platform_it};

    Signal *signal{platform->get_signal()};
    handle_signal_state(tick, signal);

    auto train_it{std::ranges::find_if(trains, [&event](Train *t)
                                       { return t->get_id() == event.train_id; })};
    if (train_it == trains.end())
    {
        std::cerr << "No matching train found for event train id: " << event.train_id << " in spawn train\n";
        return false;
    }
    Train *train{*train_it};

    if (train->is_idle())
    {
        bool spawned{train->spawn(platform)};
        if (!spawned)
        {
            return false;
        }

        logger->info(std::format("Train {} is leaving the yard {} (actual tick {}, planned tick {})", train->get_id(), station->get_name(), tick, event.tick));
        handle_signal_state(tick, signal);

        return true;
    }
    else
    {
        return false;
    }
}

void Dispatch::despawn_train(int tick, const Event &event, Train *train, const Station *yard)
{
    train->despawn();
    logger->info(std::format("Train {} is arriving at yard {} (actual tick {}, planned tick {})", train->get_id(), yard->get_name(), tick, event.tick));

    if (train->is_out_of_service())
    {
        finished_trains.insert(train);
    }

    check_completion(tick);
}

void Dispatch::check_completion(int tick)
{
    if (finished_trains.size() == trains.size())
    {
        agency_control->inactivate(this);
    }
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