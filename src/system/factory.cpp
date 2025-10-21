/**
 * for details on design, see:
 * docs/system/factory.md
 */

#include <ranges>
#include <algorithm>

#include "enum/transit_types.h"
#include "utils/utils.h"
#include "system/factory.h"

void Factory::build_network(const Transit::Map::Graph &graph, const Registry &registry, Constants::System system_code)
{
    create_trains(registry, system_code);
    create_stations(graph, registry, system_code);

    const auto &routes_map{graph.get_routes()};
    const auto &yard_registry{registry.get_yard_registry(system_code)};

    std::unordered_map<TrainLine, std::pair<int, int>> yard_map{};
    for (const auto &yard_pair : yard_registry)
    {
        Info yard_info{registry.decode(yard_pair.first)};
        yard_map[yard_info.train_line] = yard_pair;
    }

    for (const auto &[train_line, routes] : routes_map)
    {
        auto it{yard_map.find(train_line)};
        if (it == yard_map.end())
        {
            continue;
        }

        auto yard_pair{it->second};

        for (const auto &route : routes)
        {
            for (int i{1}; i < route.sequence.size(); ++i)
            {
                int from_id{route.sequence[i - 1]};
                int to_id{route.sequence[i]};

                Station *from{stations.at(from_id).get()};
                Station *to{stations.at(to_id).get()};
                int duration{route.distances[i - 1]};

                if (system_code == Constants::System::LIRR)
                {
                    std::cout << "Route to " << route.headsign << ", duration: " << duration << std::endl;
                }

                create_track(from, to, train_line, route.direction, duration);
            }

            int start_id{};
            int end_id{};

            Info first{registry.decode(yard_pair.first)};
            Info second{registry.decode(yard_pair.second)};

            if (directions_equal(route.direction, second.direction))
            {
                start_id = first.id;
                end_id = second.id;
            }
            else
            {
                start_id = second.id;
                end_id = first.id;
            }

            Station *start_yard{stations.at(start_id).get()};
            Station *end_yard{stations.at(end_id).get()};

            Station *first_station{stations.at(route.sequence.front()).get()};
            Station *last_station{stations.at(route.sequence.back()).get()};

            create_track(start_yard, first_station, train_line, route.direction, Constants::DEFAULT_TRAVEL_TIME);
            create_track(last_station, end_yard, train_line, route.direction, Constants::DEFAULT_TRAVEL_TIME);
        }
    }
}

std::vector<Train *> Factory::get_trains() const
{
    std::vector<Train *> output{};
    output.reserve(trains.size());

    std::ranges::transform(trains, std::back_inserter(output), [](const auto &pair)
                           { return pair.second.get(); });

    return output;
}

std::vector<Train *> Factory::get_trains(TrainLine train_line) const
{
    std::vector<Train *> output{};
    output.reserve(trains.size());

    for (auto &[id, train] : trains)
    {
        if (trainlines_equal(train->get_train_line(), train_line))
        {
            output.push_back(train.get());
        }
    }
    return output;
}

std::vector<Station *> Factory::get_stations() const
{
    std::vector<Station *> output{};
    output.reserve(stations.size());

    std::ranges::transform(stations, std::back_inserter(output), [](const auto &pair)
                           { return pair.second.get(); });

    return output;
}

std::vector<Station *> Factory::get_stations(TrainLine train_line) const
{
    std::vector<Station *> output{};
    output.reserve(stations.size());

    for (auto &[id, station] : stations)
    {
        if (station->get_train_lines().contains(train_line))
        {
            output.push_back(station.get());
        }
    }
    return output;
}

std::vector<Signal *> Factory::get_signals() const
{
    std::vector<Signal *> output{};
    output.reserve(signals.size());

    std::ranges::transform(signals, std::back_inserter(output), [](const auto &pair)
                           { return pair.second.get(); });

    return output;
}

std::vector<Platform *> Factory::get_platforms() const
{
    std::vector<Platform *> output{};
    output.reserve(platforms.size());

    std::ranges::transform(platforms, std::back_inserter(output), [](const auto &pair)
                           { return pair.second.get(); });

    return output;
}

std::vector<Track *> Factory::get_tracks() const
{
    std::vector<Track *> output{};
    output.reserve(tracks.size());

    std::ranges::transform(tracks, std::back_inserter(output), [](const auto &pair)
                           { return pair.second.get(); });

    return output;
}

std::vector<Switch *> Factory::get_switches() const
{
    std::vector<Switch *> output{};
    output.reserve(switches.size());

    std::ranges::transform(switches, std::back_inserter(output), [](const auto &pair)
                           { return pair.second.get(); });

    return output;
}

int Factory::generate_signal_id()
{
    return next_signal_id++;
}

int Factory::generate_track_id()
{
    return next_track_id++;
}

int Factory::generate_switch_id()
{
    return next_switch_id++;
}

void Factory::create_trains(const Registry &registry, Constants::System system_code)
{
    const auto &train_registry{registry.get_train_registry(system_code)};
    trains.reserve(train_registry.size());

    for (const auto &encoded : train_registry)
    {
        Info info{registry.decode(encoded)};

        trains.emplace(
            info.id,
            std::make_unique<Train>(
                info.id,
                info.train_line,
                ServiceType::BOTH,
                info.direction));
    }
}

void Factory::create_stations(const Transit::Map::Graph &graph, const Registry &registry, Constants::System system_code)
{
    const auto &adj_list{graph.get_adjacency_list()};
    const auto &yard_registry{registry.get_yard_registry(system_code)};
    auto directions{Constants::get_directions_by_system_code(system_code)};

    stations.reserve(adj_list.size() + (yard_registry.size() * 2));

    auto create_platforms = [&](const std::array<Direction, 2> &directions, Station *station_ptr)
    {
        for (const auto &direction : directions)
        {
            int signal_id{generate_signal_id()};
            int track_id{generate_track_id()};

            auto signal{std::make_unique<Signal>(signal_id)};
            Signal *signal_ptr{signal.get()};
            signals.emplace(signal_id, std::move(signal));

            int duration{station_ptr->is_yard() ? 0 : Constants::DEFAULT_DWELL_TIME};

            auto platform{std::make_unique<Platform>(
                track_id,
                signal_ptr,
                station_ptr,
                direction,
                duration,
                station_ptr->get_train_lines())};

            station_ptr->add_platform(platform.get());
            signal_ptr->set_track(static_cast<Track *>(platform.get()));
            platforms.emplace(track_id, std::move(platform));
        }
    };

    for (const auto &[id, edges] : adj_list)
    {
        const Transit::Map::Node *node{graph.get_node(id)};

        auto station{std::make_unique<Station>(id, node->name, false, node->train_lines)};
        create_platforms(directions, station.get());
        stations.emplace(id, std::move(station));
    }

    for (const auto &[start_id, end_id] : yard_registry)
    {
        Info start{registry.decode(start_id)};
        Info end{registry.decode(end_id)};

        auto start_yard{std::make_unique<Station>(start_id, Utils::generate_yard_name(start), true, std::unordered_set<TrainLine>{start.train_line})};
        auto end_yard{std::make_unique<Station>(end_id, Utils::generate_yard_name(end), true, std::unordered_set<TrainLine>{end.train_line})};

        create_platforms(directions, start_yard.get());
        create_platforms(directions, end_yard.get());

        stations.emplace(start_id, std::move(start_yard));
        stations.emplace(end_id, std::move(end_yard));
    }
}

void Factory::create_track(Station *from, Station *to, TrainLine train_line, Direction direction, int duration)
{
    std::optional<Platform *> from_platform_opt{from->select_platform(direction, train_line)};
    std::optional<Platform *> to_platform_opt{to->select_platform(direction, train_line)};

    if (!from_platform_opt.has_value())
    {
        std::cerr << "No valid platform found at station " << from->get_name() << " for line " << trainline_to_string(train_line) << " in direction " << direction_to_string(direction) << "\n";
        return;
    }

    if (!to_platform_opt.has_value())
    {
        std::cerr << "No valid platform found at station " << to->get_name() << " for line " << trainline_to_string(train_line) << " in direction " << direction_to_string(direction) << "\n";
        return;
    }

    Platform *from_platform{*from_platform_opt};
    Platform *to_platform{*to_platform_opt};

    auto it{connected_platforms.find(from_platform)};
    if (it != connected_platforms.end())
    {
        if (it->second.contains(to_platform))
        {
            return;
        }
    }

    // dividing track block into multiple parts as needed
    std::vector<int> duration_subparts{};
    while (duration > 0)
    {
        int remaining_parts{static_cast<int>(std::ceil(static_cast<double>(duration) / Constants::MAX_TRACK_DURATION))};
        int part{duration / remaining_parts};

        if (duration % remaining_parts != 0)
        {
            part += 1;
        }

        if (part > Constants::MAX_TRACK_DURATION)
        {
            part = Constants::MAX_TRACK_DURATION;
        }

        duration_subparts.push_back(part);
        duration -= part;
    }

    // construct and connect from track block sub parts
    Track *current{static_cast<Track *>(from_platform)};
    for (int i{0}; i < duration_subparts.size(); ++i)
    {
        int signal_id{generate_signal_id()};
        int track_id{generate_track_id()};

        auto signal{std::make_unique<Signal>(signal_id)};
        Signal *signal_ptr{signal.get()};
        signals.emplace(signal_id, std::move(signal));

        auto track{std::make_unique<Track>(track_id, signal_ptr, duration_subparts[i], std::unordered_set<TrainLine>{train_line})};
        Track *track_ptr{track.get()};
        tracks.emplace(track_id, std::move(track));

        signal_ptr->set_track(track_ptr);

        current->add_next_track(track_ptr);
        track_ptr->add_prev_track(current);
        current = track_ptr;
    }

    current->add_next_track(to_platform);
    to_platform->add_prev_track(current);

    connected_platforms[from_platform].insert(to_platform);
    create_switch(from_platform, to_platform);
}

void Factory::create_switch(Platform *from, Platform *to)
{
    const auto &next_tracks{from->get_next_tracks()};
    const auto &prev_tracks{to->get_prev_tracks()};

    if (next_tracks.size() > 1)
    {
        Switch *sw{from->get_outbound_switch()};
        if (sw == nullptr)
        {
            int switch_id{generate_switch_id()};
            auto new_sw{std::make_unique<Switch>(switch_id)};
            sw = new_sw.get();
            switches.emplace(switch_id, std::move(new_sw));

            for (Track *tr : next_tracks)
            {
                sw->add_departure_track(tr);
                tr->add_inbound_switch(sw);
            }
        }

        from->add_outbound_switch(sw);
        sw->add_approach_track(from);
    }

    if (prev_tracks.size() > 1)
    {
        Switch *sw{to->get_inbound_switch()};
        if (sw == nullptr)
        {
            int switch_id{generate_switch_id()};
            auto new_sw{std::make_unique<Switch>(switch_id)};
            sw = new_sw.get();
            switches.emplace(switch_id, std::move(new_sw));

            for (Track *tr : prev_tracks)
            {
                sw->add_approach_track(tr);
                tr->add_outbound_switch(sw);
            }
        }

        to->add_inbound_switch(sw);
        sw->add_departure_track(to);
    }
}