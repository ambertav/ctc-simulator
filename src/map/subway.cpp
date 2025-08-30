#include <fstream>
#include <string_view>
#include <unordered_set>
#include <ranges>
#include <algorithm>

#include "map/subway.h"
#include "utils/utils.h"
#include "enum/transit_types.h"
#include "config.h"

using namespace Transit::Map;

Subway::Subway()
{
    load_stations(std::string(DATA_DIRECTORY) + "/clean/subway/stations.csv");
    load_connections(std::string(DATA_DIRECTORY) + "/clean/subway/routes.csv");
}

void Subway::load_stations(const std::string &csv)
{
    const std::vector<std::string_view> needed_columns{
        "complex_id",
        "gtfs_id",
        "stop_name",
        "train_lines",
        "latitude",
        "longitude"};

    Utils::open_and_parse(csv, needed_columns, [&](std::string_view line, const std::unordered_map<std::string_view, int> &column_index, int line_num)
                          {              
        auto tokens {Utils::split(line, ',')};
        if (tokens.size() < column_index.size())
        {
            Utils::log_malformed_line("subway", "load stations", line_num, line);
            return;
        }

        const auto row {Utils::from_tokens(tokens, column_index)};

        int complex_id {Utils::string_view_to_numeric<int>(row.at("complex_id"))};
        std::string gtfs {row.at("gtfs_id")};
        std::string stop_name {row.at("stop_name")};
        std::string_view train_lines_sv {row.at("train_lines")};
        double latitude {Utils::string_view_to_numeric<double>(row.at("latitude"))};
        double longitude {Utils::string_view_to_numeric<double>(row.at("longitude"))};

        auto train_line_tokens {Utils::split(train_lines_sv, ' ')};
        std::unordered_set<TrainLine> train_lines{};
        train_lines.reserve(train_line_tokens.size());

        for (const auto &token : train_line_tokens)
        {
            train_lines.insert(trainline_from_string(std::string(token)));
        }

        try
        {
            add_node(complex_id, stop_name, train_lines, {gtfs}, latitude, longitude);
        }
        catch (const std::invalid_argument &e)
        {
            update_node(complex_id, train_lines, {gtfs});
        }

        gtfs_to_id[gtfs] = complex_id; });
}

void Subway::load_connections(const std::string &csv)
{
    const std::vector<std::string_view> needed_columns{
        "route_id",
        "headsign",
        "ordered_stops"};

    Utils::open_and_parse(csv, needed_columns, [&](std::string_view line, const std::unordered_map<std::string_view, int> &column_index, int line_num)
                          {

        auto tokens {Utils::split(line, ',')};
        if (tokens.size() < column_index.size())
        {
            Utils::log_malformed_line("subway", "load connections", line_num, line);
            return;
        }

        const auto row {Utils::from_tokens(tokens, column_index)};

        std::string route_str {row.at("route_id")};
        std::string headsign {row.at("headsign")};
        std::string ordered_stops {row.at("ordered_stops")};

        TrainLine route {trainline_from_string(std::string(route_str))};

        auto stop_ids {Utils::split(ordered_stops, ' ')};

        auto sequence_view = stop_ids
            | std::views::all
            | std::views::transform([&](std::string_view gtfs_id) -> std::optional<int>
            {
                auto it = gtfs_to_id.find(gtfs_id);
                return (it != gtfs_to_id.end() ? std::make_optional(it->second) : std::nullopt);
            })
            | std::views::filter([](const auto& opt)
            {
                return opt.has_value();
            })
            | std::views::transform([](const auto& opt)
            {
                return *opt;
            });

        std::vector<int> sequence {sequence_view.begin(), sequence_view.end()};

        std::vector<int> distances{};
        distances.reserve(sequence.size() - 1);

        for (int i = 1; i < sequence.size(); ++i)
        {
            int u {sequence[i - 1]};
            int v {sequence[i]};

            auto* edge {add_edge(u, v)};
            distances.push_back(static_cast<int>(std::ceil(edge->weight)));
        }

        add_route(route, headsign, sequence, distances); });
}