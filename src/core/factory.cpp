#include <ranges>
#include <algorithm>

#include "core/factory.h"
#include "enums/transit_types.h"

#include "utils.h"
#include "constants.h"

void Factory::build_network(int total_trains, const Transit::Map::Path &path)
{

    create_trains(total_trains, path.nodes.front()->train_lines[0]);
    auto [start, end] = create_stations(path);
    create_network(path, start, end);
}

std::vector<Train *> Factory::get_trains() const
{
    std::vector<Train *> output{};
    output.reserve(trains.size());

    std::ranges::transform(trains, std::back_inserter(output), [](const auto &pair)
                           { return pair.second.get(); });

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

void Factory::clear()
{
    trains.clear();
    stations.clear();
    signals.clear();
    platforms.clear();
    tracks.clear();
}

void Factory::create_trains(int total, TrainLine train_line)
{
    for (int i = 1; i <= total; ++i)
    {
        trains[i] = std::make_unique<Train>(i, train_line, ServiceType::BOTH, nullptr);
    }
}

std::pair<int, int> Factory::create_stations(const Transit::Map::Path &path)
{
    using namespace Transit::Map;

    for (const auto &node : path.nodes)
    {
        stations[node->id] = std::make_unique<Station>(node->id, node->name, false, node->train_lines);
    }

    const Node *start_node{path.nodes.front()};
    const Node *end_node{path.nodes.back()};

    std::pair<double, double> from{start_node->coordinates.latitude, start_node->coordinates.longitude};
    std::pair<double, double> to{end_node->coordinates.latitude, end_node->coordinates.longitude};

    std::optional<Direction> dir_opt = infer_direction(start_node->train_lines.front(), from, to);

    auto [start, end] = Yards::get_yard_id_by_direction(*dir_opt);

    stations[start] = std::make_unique<Station>(start, Yards::get_yard_name(start), true, path.nodes.front()->train_lines);
    stations[end] = std::make_unique<Station>(end, Yards::get_yard_name(end), true, path.nodes.back()->train_lines);

    return {start, end};
}

void Factory::create_network(const Transit::Map::Path &path, int start, int end)
{
    int dwell_time{2};

    static int track_id{1};
    static int signal_id{1};

    auto build = [&](const std::vector<const Transit::Map::Node *> &nodes, const std::vector<double> &distances, int start, int end, Direction dir, bool is_reversed)
    {
        std::unique_ptr<Track> *prev_track = nullptr;
        std::unique_ptr<Platform> *prev_platform = nullptr;

        // connect from start yard
        signals[signal_id] = std::make_unique<Signal>(signal_id, track_id);
        platforms[track_id] = std::make_unique<Platform>(track_id, dwell_time, signals[signal_id].get(), stations[start].get(), dir);
        stations[start]->add_platform(platforms[track_id].get());

        prev_platform = &platforms[track_id];

        ++signal_id;
        ++track_id;

        for (int i = 0; i < nodes.size(); ++i) // connect path nodes
        {
            double segment_weight{1.0};
            if (i < distances.size())
            {
                segment_weight = is_reversed
                                     ? distances[distances.size() - 1 - i]
                                     : distances[i];
            }

            signals[signal_id] = std::make_unique<Signal>(signal_id, track_id);
            tracks[track_id] = std::make_unique<Track>(track_id, signals[signal_id].get(), segment_weight);

            if (prev_platform)
            {
                (*prev_platform)->set_next(tracks[track_id].get());
                tracks[track_id]->set_prev((*prev_platform).get());
            }
            else if (prev_track)
            {
                (*prev_track)->set_next(tracks[track_id].get());
                tracks[track_id]->set_prev((*prev_track).get());
            }

            prev_track = &tracks[track_id];

            ++signal_id;
            ++track_id;

            int station_id{nodes[i]->id};
            signals[signal_id] = std::make_unique<Signal>(signal_id, track_id);
            platforms[track_id] = std::make_unique<Platform>(track_id, dwell_time, signals[signal_id].get(), stations[station_id].get(), dir);
            stations[station_id]->add_platform(platforms[track_id].get());

            if (prev_track)
            {
                (*prev_track)->set_next(platforms[track_id].get());
                platforms[track_id]->set_prev((*prev_track).get());

                prev_track = nullptr;
            }

            prev_platform = &platforms[track_id];
            ++signal_id;
            ++track_id;
        }

        // connect to end yard
        signals[signal_id] = std::make_unique<Signal>(signal_id, track_id);
        tracks[track_id] = std::make_unique<Track>(track_id, signals[signal_id].get(), 1);

        if (prev_platform)
        {
            (*prev_platform)->set_next(tracks[track_id].get());
            tracks[track_id]->set_prev((*prev_platform).get());
        }

        prev_track = &tracks[track_id];
        ++signal_id;
        ++track_id;

        // end yard platform
        signals[signal_id] = std::make_unique<Signal>(signal_id, track_id);
        platforms[track_id] = std::make_unique<Platform>(track_id, dwell_time, signals[signal_id].get(), stations[end].get(), dir);
        stations[end]->add_platform(platforms[track_id].get());

        if (prev_track)
        {
            (*prev_track)->set_next(platforms[track_id].get());
            platforms[track_id]->set_prev((*prev_track).get());
        }

        ++signal_id;
        ++track_id;
    };

    const Transit::Map::Node *start_node{path.nodes.front()};
    const Transit::Map::Node *end_node{path.nodes.back()};

    std::pair<double, double> from{start_node->coordinates.latitude, start_node->coordinates.longitude};
    std::pair<double, double> to{end_node->coordinates.latitude, end_node->coordinates.longitude};

    std::optional<Direction> o_opt = infer_direction(start_node->train_lines.front(), from, to);
    std::optional<Direction> r_opt = infer_direction(start_node->train_lines.front(), to, from);

    Direction original{*o_opt};
    Direction reverse{*r_opt};

    const std::vector<const Transit::Map::Node *> reversed_path(path.nodes.rbegin(), path.nodes.rend());
    const std::vector<double> reversed_distances(path.segment_weights.rbegin(), path.segment_weights.rend());

    build(path.nodes, path.segment_weights, start, end, original, false);
    build(reversed_path, reversed_distances, end, start, reverse, true);
}
