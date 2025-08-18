#include <queue>
#include <array>
#include <iostream>

#include "config.h"
#include "utils/utils.h"
#include "constants/railroad_constants.h"

#include "map/lirr.h"

using namespace Transit::Map;

LongIslandRailroad::LongIslandRailroad()
{
    load_stations(std::string(DATA_DIRECTORY) + "/clean/lirr/stations.csv");
    load_connections(std::string(DATA_DIRECTORY) + "/clean/lirr/routes.csv");
}

void LongIslandRailroad::load_stations(const std::string &csv)
{
    const std::vector<std::string_view> needed_columns{
        "stop_id",
        "stop_code",
        "stop_name",
        "latitude",
        "longitude"};

    Utils::open_and_parse(csv, needed_columns, [&](std::string_view line, const std::unordered_map<std::string_view, int> &column_index, int line_num)
                          {
    auto tokens {Utils::split(line, ',')};
    if (tokens.size() < column_index.size())
    {
        Utils::log_malformed_line("lirr", "load stations", line_num, line);
        return;
    }

    const auto row {Utils::from_tokens(tokens, column_index)};

    int stop_id {Utils::string_view_to_numeric<int>(row.at("stop_id"))};
    std::string stop_code {row.at("stop_code")};
    std::string stop_name {row.at("stop_name")};
    double latitude {Utils::string_view_to_numeric<double>(row.at("latitude"))};
    double longitude {Utils::string_view_to_numeric<double>(row.at("longitude"))};

    // will add train lines in load_connections()
    add_node(stop_id, stop_name, {}, {stop_code}, latitude, longitude); });
}

void LongIslandRailroad::load_connections(const std::string &csv)
{
    const std::vector<std::string_view> needed_columns{
        "route_id",
        "ordered_stops"};

    std::unordered_map<std::string_view, std::vector<std::vector<int>>> route_segments{};

    Utils::open_and_parse(csv, needed_columns, [&](std::string_view line, const std::unordered_map<std::string_view, int> &column_index, int line_num)
                          {
        auto tokens {Utils::split(line, ',')};
        if (tokens.size() < column_index.size())
        {
            Utils::log_malformed_line("lirr", "load connections", line_num, line);
            return;
        }

        const auto row {Utils::from_tokens(tokens, column_index)};

        std::string_view route_sv {row.at("route_id")};
        std::string_view ordered_stops_sv {row.at("ordered_stops")};

        auto stop_tokens {Utils::split(ordered_stops_sv, ' ')};

        std::vector<int> stop_ids{};
        stop_ids.reserve(stop_tokens.size());
        for (const auto& stop_sv : stop_tokens)
        {
            stop_ids.push_back(Utils::string_view_to_numeric<int>(stop_sv));
        }

        route_segments[route_sv].push_back(std::move(stop_ids)); });

    for (const auto &[route_sv, segments] : route_segments)
    {
        if (route_sv == "City Terminal Zone")
        {
            continue;
        }

        TrainLine route{trainline_from_string(std::string(route_sv))};
        auto merged = merge_segments(segments);

        std::array<std::pair<int, size_t>, 4> terminal_positions{};
        size_t terminal_count{};

        for (size_t i = 0; i < merged.size(); ++i)
        {
            if (LIRR::city_terminal_stations.contains(merged[i]))
            {
                if (merged[i] == 102 && route_sv != "Oyster Bay Branch") // skips so that Jamaica is only terminal for Oyster Bay
                {
                    continue;
                }

                terminal_positions[terminal_count++] = {merged[i], i};
                if (terminal_count == 4)
                {
                    break;
                }
            }
        }

        if (terminal_count == 0)
        {
            std::cout << "No terminal stations found for " << route_sv << ", skipping...\n";
            continue;
        }

        if (terminal_positions[0].second == 0)
        {
            std::cout << "No branch stations found for " << route_sv << ", skipping...\n";
            continue;
        }

        size_t branch_point_position{terminal_positions[0].second - 1}; // to get all stops before first terminal station

        for (size_t i = 0; i < terminal_count; ++i)
        {
            const auto &[terminal_id, position] = terminal_positions[i];

            std::string city_headsign{LIRR::city_terminal_stations.at(terminal_id)};
            std::vector<int> westbound_sequence(merged.begin(), merged.begin() + branch_point_position + 1);
            westbound_sequence.push_back(merged[position]);

            add_route(route, city_headsign, westbound_sequence);

            for (int j = 1; j < westbound_sequence.size(); ++j)
            {
                int u{westbound_sequence[j - 1]};
                int v{westbound_sequence[j]};

                update_node(u, {route}, {});
                update_node(v, {route}, {});

                add_edge(u, v);
            }

            std::vector<int> eastbound_sequence(westbound_sequence.rbegin(), westbound_sequence.rend());
            std::string li_headsign{LIRR::li_terminal_stations.at(merged[0])};

            add_route(route, li_headsign, eastbound_sequence);
        }
    }
}

std::vector<int> LongIslandRailroad::merge_segments(const std::vector<std::vector<int>> &segments)
{
    if (segments.empty())
    {
        return {};
    }

    if (segments.size() == 1)
    {
        return segments[0];
    }

    std::unordered_map<int, std::unordered_set<int>> precedence{};
    std::unordered_set<int> all_stops{};

    for (const auto &segment : segments)
    {
        for (int i = 0; i < segment.size(); ++i)
        {
            all_stops.insert(segment[i]);
            for (int j = i + 1; j < segment.size(); ++j)
            {
                precedence[segment[i]].insert(segment[j]);
            }
        }
    }

    std::vector<std::vector<int>> valid_segments{};
    for (const auto &segment : segments)
    {
        if (!segment.empty())
        {
            valid_segments.push_back(segment);
        }
    }

    return k_way_merge(valid_segments, precedence);
}

std::vector<int> LongIslandRailroad::k_way_merge(const std::vector<std::vector<int>> &segments, const std::unordered_map<int, std::unordered_set<int>> &precedence)
{
    using PQElement = std::pair<int, int>;
    auto comparator = [&precedence](const PQElement &a, const PQElement &b)
    {
        int stop_a{a.first};
        int stop_b{b.first};

        if (precedence.count(stop_a) && precedence.at(stop_a).count(stop_b))
        {
            return false;
        }

        if (precedence.count(stop_b) && precedence.at(stop_b).count(stop_a))
        {
            return true;
        }

        return stop_a > stop_b;
    };

    std::priority_queue<PQElement, std::vector<PQElement>, decltype(comparator)> pq(comparator);
    std::vector<int> indices(segments.size(), 0);

    for (int i = 0; i < segments.size(); ++i)
    {
        if (!segments[i].empty())
        {
            pq.emplace(segments[i][0], i);
        }
    }

    std::vector<int> result{};
    std::unordered_set<int> seen{};

    while (!pq.empty())
    {
        auto [stop, segment_index] = pq.top();
        pq.pop();

        if (!seen.count(stop))
        {
            result.push_back(stop);
            seen.insert(stop);
        }

        ++indices[segment_index];

        if (indices[segment_index] < segments[segment_index].size())
        {
            int next_stop{segments[segment_index][indices[segment_index]]};
            pq.emplace(next_stop, segment_index);
        }
    }

    return result;
}