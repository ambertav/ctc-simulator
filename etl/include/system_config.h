#pragma once

#include <string>
#include <string_view>
#include <optional>

namespace etl
{
    struct SystemConfig
    {
        std::string name;

        std::string station_input_file;
        std::string trips_input_file;
        std::string stop_times_input_file;
        std::optional<std::string> routes_input_file;

        std::string station_output_file;
        std::string routes_output_file;

        std::string station_header;
        std::string routes_header;

        std::vector<std::string_view> station_columns;
        std::vector<std::string_view> trip_columns;
        std::vector<std::string_view> stop_time_columns;
        std::optional<std::vector<std::string_view>> route_columns;

        std::function<bool(const std::unordered_map<std::string_view, std::string_view>&)> trip_filter;
        std::function<std::vector<std::string>(const std::vector<std::pair<int, std::string>> &)> transform_sequence;

        bool multiple_trips;


    };
}