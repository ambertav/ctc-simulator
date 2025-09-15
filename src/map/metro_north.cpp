/**
 * for details on design, see:
 * docs/map/metro_north.md
 */

#include <queue>

#include "config.h"
#include "utils/utils.h"
#include "constants/railroad_constants.h"
#include "map/metro_north.h"

using namespace Transit::Map;

MetroNorth::MetroNorth()
{
    load_stations(std::string(DATA_DIRECTORY) + "/clean/mnr/stations.csv");
    load_connections(std::string(DATA_DIRECTORY) + "/clean/mnr/routes.csv");
}

void MetroNorth::load_stations(const std::string &csv)
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
            Utils::log_malformed_line("metro north", "load stations", line_num, line);
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

void MetroNorth::load_connections(const std::string &csv)
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
            Utils::log_malformed_line("metro north", "load connections", line_num, line);
            return;
        }

        const auto row {Utils::from_tokens(tokens, column_index)};

        std::string_view route_sv {row.at("route_id")};
        std::string_view ordered_stops_sv{row.at("ordered_stops")};

        auto stop_tokens {Utils::split(ordered_stops_sv, ' ')};

        std::vector<int> stop_ids{};
        stop_ids.reserve(stop_tokens.size());
        for (const auto &stop_sv : stop_tokens)
        {
            stop_ids.push_back(Utils::string_view_to_numeric<int>(stop_sv));
        }
        route_segments[route_sv].push_back(std::move(stop_ids)); });

    for (const auto &[route_sv, segments] : route_segments)
    {
        std::string route_str{std::string(route_sv)};
        if (route_sv == "Harlem" || route_sv == "Hudson" || route_sv == "New Haven")
        {
            TrainLine route{trainline_from_string(route_str)};
            auto merged = merge_segments(segments);

            std::string outbound_headsign{};
            if (route_sv == "Harlem")
            {
                outbound_headsign = "Wassaic";
            }
            else if (route_sv == "Hudson")
            {
                outbound_headsign = "Poughkeepsie";
            }
            else if (route_sv == "New Haven")
            {
                outbound_headsign = "New Haven-State St";
            }

            std::vector<int> outbound_distances{};
            outbound_distances.reserve(merged.size() - 1);

            for (int i = 1; i < merged.size(); ++i)
            {
                int u{merged[i - 1]};
                int v{merged[i]};

                // adds train lines
                update_node(u, {route}, {});
                update_node(v, {route}, {});

                auto* edge {add_edge(u, v)};
                outbound_distances.push_back(static_cast<int>(std::ceil(edge->weight)));
            }

            add_route(route, outbound_headsign, merged, outbound_distances);

            std::vector<int> inbound_sequence(merged.rbegin(), merged.rend());
            std::vector<int> inbound_distances(outbound_distances.rbegin(), outbound_distances.rend());
            add_route(route, "Grand Central", inbound_sequence, inbound_distances);
        }
        else if (route_sv == "New Canaan" || route_sv == "Danbury" || route_sv == "Waterbury")
        {
            TrainLine route{trainline_from_string(route_str)};
            auto branch_segment = handle_branches(route_sv, segments);

            std::vector<int> outbound_distances{};
            outbound_distances.reserve(branch_segment.size() - 1);

            for (int i = 1; i < branch_segment.size(); ++i)
            {
                int u{branch_segment[i - 1]};
                int v{branch_segment[i]};

                // adds train lines
                update_node(u, {route}, {});
                update_node(v, {route}, {});

                auto* edge {add_edge(u, v)};
                outbound_distances.push_back(static_cast<int>(std::ceil(edge->weight)));
            }

            add_route(route, route_str, branch_segment, outbound_distances);


            std::vector<int> inbound_branch(branch_segment.rbegin(), branch_segment.rend());
            std::vector<int> inbound_distances(outbound_distances.rbegin(), outbound_distances.rend());


            std::string branch_point{get_branch_point_name(route_sv)};
            add_route(route, branch_point, inbound_branch, inbound_distances);
        }
    }
}

std::vector<int> MetroNorth::merge_segments(const std::vector<std::vector<int>> &segments)
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

std::vector<int> MetroNorth::k_way_merge(const std::vector<std::vector<int>> &segments, const std::unordered_map<int, std::unordered_set<int>> &precedence)
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

std::vector<int> MetroNorth::handle_branches(std::string_view branch_name, const std::vector<std::vector<int>> &segments)
{
    auto full_route = merge_segments(segments);

    int branch_point{get_branch_point(branch_name)};
    if (branch_point == -1)
    {
        return {};
    }

    auto it = std::find(full_route.begin(), full_route.end(), branch_point);
    if (it == full_route.end())
    {
        return {};
    }

    std::vector<int> branch{};
    branch.insert(branch.end(), it, full_route.end());
    return branch;
}

int MetroNorth::get_branch_point(std::string_view branch_name) const
{
    auto it = MNR::branch_data.find(branch_name);
    return it != MNR::branch_data.end() ? it->second.stop_id : -1;
}

std::string_view MetroNorth::get_branch_point_name(std::string_view branch_name) const
{
    auto it = MNR::branch_data.find(branch_name);
    return it != MNR::branch_data.end() ? it->second.name : "Junction";
}