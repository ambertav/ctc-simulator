#include <queue>
#include <fstream>
#include <iostream>

#include "config.h"
#include "utils.h"
#include "railroad_constants.h"
#include "map/metro_north.h"

using namespace Transit::Map;

MetroNorth::MetroNorth()
{
    load_stations(std::string(DATA_DIRECTORY) + "/clean/mnr/stations.csv");
    load_connections(std::string(DATA_DIRECTORY) + "/clean/mnr/routes.csv");
}

void MetroNorth::load_stations(const std::string &csv)
{
    const std::vector<std::string> needed_columns{
        "stop_id",
        "stop_name",
        "latitude",
        "longitude"};

    Utils::open_and_parse(csv, needed_columns, [&](std::ifstream &file, const std::unordered_map<std::string, int> &column_index)
                          {
    std::string line;
    int line_num{};
    while (std::getline(file, line))
    {
        ++line_num;
        auto tokens = Utils::split(line, ',');
        if (tokens.size() < column_index.size())
        {
            std::cerr << "In metro north stops.txt, malformed line " << line_num << ": " << line << "\n";
            continue;
        }

        int stop_id {std::stoi(tokens[column_index.at("stop_id")])};
        std::string stop_name {tokens[column_index.at("stop_name")]};
        std::string lat_str {tokens[column_index.at("latitude")]};
        std::string lon_str {tokens[column_index.at("longitude")]};

        double latitude {std::stod(lat_str)};
        double longitude {std::stod(lon_str)};

        // will add train lines in load_connections()
        add_node(stop_id, stop_name, {}, {}, latitude, longitude);
    } });
}

void MetroNorth::load_connections(const std::string &csv)
{
    const std::vector<std::string> needed_columns{
        "route_id",
        "headsign",
        "ordered_stops"};

    Utils::open_and_parse(csv, needed_columns, [&](std::ifstream &file, const std::unordered_map<std::string, int> &column_index)
                          {
                              std::unordered_map<std::string, std::vector<std::vector<int>>> route_segments;

                              std::string line{};
                              int line_num{};

                              while (std::getline(file, line))
                              {
                                  ++line_num;

                                  auto tokens = Utils::split(line, ',');
                                  if (tokens.size() < column_index.size())
                                  {
                                      std::cerr << "In metro north load connections, malformed line " << line_num << ": " << line << "\n";
                                      continue;
                                  }

                                  std::string route_str{tokens[column_index.at("route_id")]};
                                  std::string headsign{tokens[column_index.at("headsign")]};
                                  std::string ordered_stops{tokens[column_index.at("ordered_stops")]};

                                  auto stop_tokens = Utils::split(ordered_stops, ' ');

                                  std::vector<int> stop_ids{};
                                  stop_ids.reserve(stop_tokens.size());
                                  for (const auto &stop_str : stop_tokens)
                                  {
                                      stop_ids.push_back(std::stoi(stop_str));
                                  }
                                  route_segments[route_str].push_back(std::move(stop_ids));
                              }

                              for (const auto &[route_name, segments] : route_segments)
                              {
                                  if (route_name == "Harlem" || route_name == "Hudson" || route_name == "New Haven")
                                  {
                                      TrainLine route{trainline_from_string(route_name)};
                                      auto merged = merge_segments(segments);

                                      std::string outbound_headsign{};
                                      if (route_name == "Harlem")
                                      {
                                          outbound_headsign += "Wassaic";
                                      }
                                      else if (route_name == "Hudson")
                                      {
                                          outbound_headsign += "Poughkeepsie";
                                      }
                                      else if (route_name == "New Haven")
                                      {
                                          outbound_headsign += "New Haven-State St";
                                      }

                                      routes[route].emplace_back(outbound_headsign, merged);

                                      for (int i = 1; i < merged.size(); ++i)
                                      {
                                          int u{merged[i - 1]};
                                          int v{merged[i]};

                                          // adds train lines
                                          update_node(u, {route}, {});
                                          update_node(v, {route}, {});

                                          add_edge(u, v);
                                      }

                                      std::vector<int> inbound_sequence(merged.rbegin(), merged.rend());
                                      routes[route].emplace_back("Grand Central", inbound_sequence);
                                  }
                                  else if (route_name == "New Canaan" || route_name == "Danbury" || route_name == "Waterbury")
                                  {
                                      TrainLine route{trainline_from_string(route_name)};
                                      auto branch_segment = handle_branches(route_name, segments);

                                      routes[route].emplace_back(route_name, branch_segment);

                                      for (int i = 1; i < branch_segment.size(); ++i)
                                      {
                                          int u{branch_segment[i - 1]};
                                          int v{branch_segment[i]};

                                          // adds train lines
                                          update_node(u, {route}, {});
                                          update_node(v, {route}, {});

                                          add_edge(u, v);
                                      }

                                      std::vector<int> inbound_branch(branch_segment.rbegin(), branch_segment.rend());

                                      routes[route].emplace_back(std::string(get_branch_point_name(route_name)), inbound_branch);
                                  }
                              } });
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

std::vector<int> MetroNorth::handle_branches(const std::string &branch_name, const std::vector<std::vector<int>> &segments)
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

std::unordered_map<TrainLine, std::vector<Route>> MetroNorth::get_routes() const
{
    return routes;
}