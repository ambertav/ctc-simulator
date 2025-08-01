#pragma once

#include <algorithm>
#include <ranges>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "config.h"
#include "system_config.h"
#include "utils.h"

namespace etl
{

    inline void process_stations(const SystemConfig &config)
    {
        std::ofstream out(config.station_output_file);
        if (!out.is_open())
        {
            Utils::log_file_open_error(config.name, "stations", config.station_output_file);
            return;
        }

        out << config.station_header << "\n";

        Utils::open_and_parse(config.station_input_file, config.station_columns, [&](std::string_view line, const std::unordered_map<std::string_view, int> &column_index, int line_num)
                              {
                auto tokens = Utils::split(line, ',');
                if (tokens.size() < column_index.size())
                {
                    Utils::log_malformed_line(config.name, "stations", line_num, line);
                    return;
                }

                auto row {Utils::from_tokens(tokens, column_index)};

                bool first = true;
            for (const auto& column : config.station_columns)
            {
                if (!first) out << ",";
                out << row[column];
                first = false;
            }
            out << "\n"; });

        out.flush();
        out.close();
    }

    inline void process_routes(const SystemConfig &config)
    {
        auto extract_headsign_to_trip = [](const SystemConfig &config)
        {
            std::unordered_map<std::string, std::vector<std::string>> headsign_to_trip{};

            Utils::open_and_parse(config.trips_input_file, config.trip_columns, [&](std::string_view line, const std::unordered_map<std::string_view, int> &column_index, int line_num)
                                  {
            std::unordered_set<std::string> used_pairs{};
            auto tokens = Utils::split(line, ',');
             if (tokens.size() < column_index.size())
                {
                    Utils::log_malformed_line(config.name, "trips", line_num, line);
                    return;
                }
            
            auto row {Utils::from_tokens(tokens, column_index)};

            if (!config.trip_filter(row))
            {
                return;
            }

            std::ostringstream oss;
            oss << row.at("route_id") << '|' << row.at("trip_headsign");
            std::string unique_key = oss.str();

            bool first_occurrence {used_pairs.insert(unique_key).second};
            if (config.multiple_trips || first_occurrence)
            {
                headsign_to_trip[unique_key].push_back(std::string(row.at("trip_id")));
            } });
            return headsign_to_trip;
        };

        auto extract_trip_to_stops = [](const SystemConfig &config, std::unordered_set<std::string> &valid_trip_ids)
        {
            std::unordered_map<std::string, std::vector<std::pair<int, std::string>>> trip_to_stops{};

            Utils::open_and_parse(config.stop_times_input_file, config.stop_time_columns, [&](std::string_view line, const std::unordered_map<std::string_view, int> &column_index, int line_num)
                                  {
            auto tokens = Utils::split(line, ',');
                         if (tokens.size() < column_index.size())
                {
                    Utils::log_malformed_line(config.name, "stop times", line_num, line);
                    return;
                }
            
            auto row {Utils::from_tokens(tokens, column_index)};

            std::string trip_id {row.at("trip_id")};
            if (valid_trip_ids.find(trip_id) == valid_trip_ids.end())
            {
                return;
            }

            int stop_seq {Utils::string_view_to_numeric<int>(row.at("stop_sequence"))};

            trip_to_stops[trip_id].emplace_back(stop_seq, row.at("stop_id")); });

            return trip_to_stops;
        };

        auto extract_route_map = [](const SystemConfig &config)
        {
            std::unordered_map<int, std::string> route_map{};

            if (!config.routes_input_file.has_value() || !config.route_columns.has_value())
            {
                return route_map;
            }

            auto routes_file{*config.routes_input_file};
            auto route_columns{*config.route_columns};

            Utils::open_and_parse(routes_file, route_columns, [&](std::string_view line, const std::unordered_map<std::string_view, int> &column_index, int line_num)
                                  {
            auto tokens = Utils::split(line, ',');
                                     if (tokens.size() < column_index.size())
                {
                    Utils::log_malformed_line(config.name, "routes", line_num, line);
                    return;
                }
            
                auto row {Utils::from_tokens(tokens, column_index)};

                int route_id {Utils::string_view_to_numeric<int>(row.at("route_id"))};

                route_map[route_id] = row.at("route_long_name"); });

            return route_map;
        };

        auto headsign_to_trip = extract_headsign_to_trip(config);

        std::unordered_set<std::string> valid_trip_ids{};
        for (const auto &[headsign, trips] : headsign_to_trip)
        {
            std::ranges::for_each(trips, [&](auto const &trip_id)
                                  { valid_trip_ids.insert(trip_id); });
        }

        auto trip_to_stops = extract_trip_to_stops(config, valid_trip_ids);
        auto route_map = extract_route_map(config);

        std::ofstream out(config.routes_output_file);
        if (!out.is_open())
        {
            Utils::log_file_open_error(config.name, "routes", config.routes_output_file);
            return;
        }

        out << config.routes_header << "\n";

        for (const auto &[route_headsign, trip_ids] : headsign_to_trip)
        {

            std::vector<std::pair<int, std::string>> sorted_stops{};

            if (config.multiple_trips)
            {
                std::string longest_trip_id{};
                size_t longest_length{};

                for (const std::string &trip_id : trip_ids)
                {
                    auto it = trip_to_stops.find(trip_id);
                    if (it == trip_to_stops.end())
                    {
                        continue;
                    }

                    if (it->second.size() > longest_length)
                    {
                        longest_length = it->second.size();
                        longest_trip_id = trip_id;
                    }
                }

                if (longest_trip_id.empty())
                {
                    continue;
                }

                sorted_stops = trip_to_stops[longest_trip_id];
            }
            else
            {
                auto it = trip_to_stops.find(trip_ids.front());
                if (it == trip_to_stops.end())
                {
                    continue;
                }

                sorted_stops = it->second;
            }

            std::ranges::sort(sorted_stops);
            auto sequence = config.transform_sequence(sorted_stops);

            auto tokens = Utils::split(route_headsign, '|');
            if (tokens.size() < 2)
            {
                continue;
            }

            std::string route_id{tokens[0]};
            std::string headsign{tokens[1]};

            std::string ordered_stops{};
            for (int i = 0; i < sequence.size(); ++i)
            {
                if (i > 0)
                {
                    ordered_stops += " ";
                }

                ordered_stops += sequence[i];
            }

            std::string route{};
            if (!route_map.empty())
            {
                int id{std::stoi(route_id)};
                auto it = route_map.find(id);
                route = (it != route_map.end() ? it->second : std::to_string(id));
            }
            else
            {
                route = route_id;
            }

            out << route << "," << headsign << "," << ordered_stops << "\n";
        }

        out.flush();
        out.close();
    }
}