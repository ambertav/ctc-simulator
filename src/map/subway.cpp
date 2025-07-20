#include <fstream>
#include <unordered_set>
#include <ranges>
#include <algorithm>

#include "map/subway.h"
#include "utils.h"
#include "enums/transit_types.h"
#include "config.h"

using namespace Transit::Map;

Subway::Subway()
{
    load_stations(std::string(DATA_DIRECTORY) + "/clean/subway/stations.csv");
    load_connections(std::string(DATA_DIRECTORY) + "/clean/subway/routes.csv");
}

void Subway::load_stations(const std::string &csv)
{
    const std::vector<std::string> needed_columns{
        "complex_id",
        "gtfs_id",
        "stop_name",
        "train_lines",
        "latitude",
        "longitude"};

    Utils::open_and_parse(csv, needed_columns, [&](std::ifstream &file, const std::unordered_map<std::string, int> &column_index)
                          {
    std::string line{};
    int line_num{};
    while (std::getline(file, line))
    {
        ++line_num;
        auto tokens = Utils::split(line, ',');
        if (tokens.size() < column_index.size())
        {
            std::cerr << "In subway stations, malformed line " << line_num << ": " << line << "\n";
            continue;
        }

        int complex_id {std::stoi(tokens[column_index.at("complex_id")])};
        std::string gtfs {tokens[column_index.at("gtfs_id")]};
        std::string stop_name {tokens[column_index.at("stop_name")]};
        std::string train_lines_str {tokens[column_index.at("train_lines")]};
        std::string latitude_str {tokens[column_index.at("latitude")]};
        std::string longitude_str {tokens[column_index.at("longitude")]};

        std::vector<std::string> train_line_tokens = Utils::split(train_lines_str, ' ');
        std::vector<TrainLine> train_lines;
        for (const auto &token : train_line_tokens)
        {
            train_lines.emplace_back(trainline_from_string(token));
        }

        double latitude {std::stod(latitude_str)};
        double longitude {std::stod(longitude_str)};

        try
        {
            Node *node = add_node(complex_id, stop_name, train_lines, {gtfs}, latitude, longitude);
        }
        catch (const std::invalid_argument &e)
        {
            update_node(complex_id, train_lines, {gtfs});
        }

        gtfs_to_id[gtfs] = complex_id;
    } });
}

void Subway::load_connections(const std::string &csv)
{
    const std::vector<std::string> needed_columns{
        "route_id",
        "headsign",
        "ordered_stops"};

    Utils::open_and_parse(csv, needed_columns, [&](std::ifstream &file, const std::unordered_map<std::string, int> &column_index)
                          {
        std::string line{};
        int line_num{};

        while (std::getline(file, line))
        {
            ++line_num;

            auto tokens = Utils::split(line, ',');
            if (tokens.size() < column_index.size())
            {
                std::cerr << "In subway load connections, malformed line " << line_num << ": " << line << "\n";
                continue;
            }

        std::string route_str {tokens[column_index.at("route_id")]};
        std::string headsign {tokens[column_index.at("headsign")]};
        std::string ordered_stops {tokens[column_index.at("ordered_stops")]};

        TrainLine route {trainline_from_string(route_str)};

        auto stop_ids = Utils::split(ordered_stops, ' ');

        auto sequence_view = stop_ids
            | std::views::filter([&](const std::string& gtfs_id)
        {
            return gtfs_to_id.contains(gtfs_id);
        })
        | std::views::transform([&](const std::string& gtfs_id)
    {
        return gtfs_to_id.at(gtfs_id);
    });

    std::vector<int> sequence {sequence_view.begin(), sequence_view.end()};

        for (int i = 1; i < sequence.size(); ++i)
        {
            int u {sequence[i - 1]};
            int v {sequence[i]};

            add_edge(u, v);
        }

        routes[route].emplace_back(headsign, sequence);
        } });
}

std::unordered_map<TrainLine, std::vector<Route>> Subway::get_routes() const
{
    return routes;
}
