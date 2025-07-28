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

        using Row = std::unordered_map<std::string_view, std::string_view>;
        Row from_tokens(const std::vector<std::string_view>& tokens, const std::unordered_map<std::string_view, int>& column_index) const
        {
            Row row{};
            for (auto & [column, index] : column_index)
            {
                if (index < tokens.size())
                {
                    row[column] = tokens[index];
                }
            }

            return row;
        }
        std::function<bool(const Row&)> trip_filter;
       std::function<std::vector<std::string>(const std::vector<std::pair<int, std::string>> &)> transform_sequence;

        bool multiple_trips;


    };
}