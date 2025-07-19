#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

#include "map/subway.h"
#include "utils.h"
#include "enums/transit_types.h"
#include "config.h"

using namespace Transit::Map;

Subway::Subway()
{
    load_stations(std::string(DATA_DIRECTORY) + "/mta_subway_stations.csv");
    load_connections(std::string(DATA_DIRECTORY) + "/gtfs_subway/trips.txt", std::string(DATA_DIRECTORY) + "/gtfs_subway/stop_times.txt");
}

void Subway::load_stations(const std::string &csv)
{
    std::ifstream file(csv);

    if (!file.is_open())
    {
        std::cerr << "Failed to open stations file\n";
        return;
    }

    std::string header;
    if (!std::getline(file, header))
    {
        std::cerr << "Stations data is empty or missing header: " << csv << "\n";
        return;
    }

    auto headers = Utils::split(header, ',');
    std::unordered_map<std::string, int> column_index;

    for (int i = 0; i < headers.size(); ++i)
    {
        column_index[headers[i]] = i;
    }

    const std::vector<std::string> needed_columns{
        "GTFS Stop ID",
        "Complex ID",
        "Stop Name",
        "Daytime Routes",
        "GTFS Latitude",
        "GTFS Longitude"};

    for (const auto &column : needed_columns)
    {
        if (column_index.find(column) == column_index.end())
        {
            std::cerr << "Missing column " << column << "\n";
            return;
        }
    }

    std::string line;
    int line_num{1};
    while (std::getline(file, line))
    {
        ++line_num;
        auto tokens = Utils::split(line, ',');
        if (tokens.size() < headers.size())
        {
            std::cerr << "Malformed line " << line_num << ": " << line << "\n";
            continue;
        }

        std::string gtfs = tokens[column_index["GTFS Stop ID"]];
        std::string complex_id = tokens[column_index["Complex ID"]];
        std::string stop_name = tokens[column_index["Stop Name"]];
        std::string daytime_routes_str = tokens[column_index["Daytime Routes"]];
        std::string longitude_str = tokens[column_index["GTFS Longitude"]];
        std::string latitude_str = tokens[column_index["GTFS Latitude"]];

        std::vector<std::string> train_line_tokens = Utils::split(daytime_routes_str, ' ');
        std::vector<TrainLine> train_lines;
        for (const auto &token : train_line_tokens)
        {
            train_lines.emplace_back(trainline_from_string(token));
        }

        double latitude = std::stod(latitude_str);
        double longitude = std::stod(longitude_str);

        try
        {
            Node *node = add_node(complex_id, stop_name, train_lines, {gtfs}, latitude, longitude);
        }
        catch (const std::invalid_argument &e)
        {
            update_node(complex_id, train_lines, {gtfs});
        }

        gtfs_to_id[gtfs] = complex_id;
    }
}

void Subway::load_connections(const std::string &trips, const std::string &stop_times)
{
    auto headsign_to_trip = extract_headsign_to_trip(trips);

    std::unordered_set<std::string> valid_trip_ids;
    for (const auto &[_, trip_id] : headsign_to_trip)
    {
        valid_trip_ids.insert(trip_id);
    }

    auto trip_to_stops = extract_trip_to_stops(stop_times, valid_trip_ids);

    for (const auto &[route_headsign, trip_id] : headsign_to_trip)
    {
        if (!trip_to_stops.count(trip_id))
        {
            continue;
        }

        auto sorted_stops = trip_to_stops[trip_id];
        std::sort(sorted_stops.begin(), sorted_stops.end());

        std::vector<std::string> sequence;

        for (const auto &[seq, gtfs_id] : sorted_stops)
        {
            sequence.push_back(gtfs_id);
        }

        for (int i = 1; i < sequence.size(); ++i)
        {
            std::string u_gtfs = sequence[i - 1];
            std::string v_gtfs = sequence[i];

            u_gtfs = u_gtfs.substr(0, u_gtfs.size() - 1);
            v_gtfs = v_gtfs.substr(0, v_gtfs.size() - 1);

            if (gtfs_to_id.find(u_gtfs) == gtfs_to_id.end() || gtfs_to_id.find(v_gtfs) == gtfs_to_id.end())
            {
                continue;
            }

            add_edge(gtfs_to_id[u_gtfs], gtfs_to_id[v_gtfs]);
        }

        add_route(route_headsign, sequence);
    }
}

void Subway::add_route(const std::string &route_headsign, std::vector<std::string> &gtfs_sequence)
{
    auto tokens = Utils::split(route_headsign, '|');

    TrainLine route = trainline_from_string(tokens[0]);
    std::string headsign = tokens[1];

    std::vector<std::string> sequence;
    for (const auto &gtfs_id : gtfs_sequence)
    {
        auto it = gtfs_to_id.find(gtfs_id);
        if (it != gtfs_to_id.end())
        {
            sequence.push_back(it->second);
        }
    }

    routes[route].emplace_back(headsign, sequence);
}

std::unordered_map<std::string, std::string> Subway::extract_headsign_to_trip(const std::string &trips)
{
    std::unordered_map<std::string, std::string> headsign_to_trip;
    const std::vector<std::string> needed_columns{
        "route_id",
        "trip_id",
        "service_id",
        "trip_headsign"};

    open_and_parse(trips, needed_columns, [&](std::ifstream &file, const std::unordered_map<std::string, int> &column_index)
                   {

    std::unordered_set<std::string> seen_headsigns;
    std::unordered_set<std::string> used_pairs;

    std::string line;
    int line_num{1};

    while (std::getline(file, line))
    {
        ++line_num;
        auto tokens = Utils::split(line, ',');

        if (tokens.size() < column_index.size())
        {
            std::cerr << "In trips.txt, malformed line " << line_num << ": " << line << "\n";
            continue;
        }

        std::string route_id = tokens[column_index.at("route_id")];
        std::string trip_id = tokens[column_index.at("trip_id")];
        std::string service_id = tokens[column_index.at("service_id")];
        std::string trip_headsign = tokens[column_index.at("trip_headsign")];

        if (service_id != "Saturday")
        {
            continue;
        }

        if (!route_id.empty() && route_id.back() == 'X')
        {
            continue;
        }

        std::string unique_key = route_id + "|" + trip_headsign;
        if (used_pairs.find(unique_key) == used_pairs.end())
        {
            headsign_to_trip[unique_key] = trip_id;
            used_pairs.insert(unique_key);
        }
    } });

    return headsign_to_trip;
}

std::unordered_map<std::string, std::vector<std::pair<int, std::string>>> Subway::extract_trip_to_stops(const std::string &stop_times, const std::unordered_set<std::string> &valid_trip_ids)
{

    std::unordered_map<std::string, std::vector<std::pair<int, std::string>>> trip_to_stops;
    const std::vector<std::string> needed_columns{
        "trip_id",
        "stop_id",
        "stop_sequence"};

    open_and_parse(stop_times, needed_columns, [&](std::ifstream &file, const std::unordered_map<std::string, int> &column_index)
                   {


    std::string line;
    int line_num{1};

    while (std::getline(file, line))
    {
        ++line_num;

        auto tokens = Utils::split(line, ',');
        if (tokens.size() < column_index.size())
        {
            std::cerr << "In stop_times.txt, malformed line " << line_num << ": " << line << "\n";
            continue;
        }

        std::string trip_id = tokens[column_index.at("trip_id")];
        if (valid_trip_ids.find(trip_id) == valid_trip_ids.end())
        {
            continue;
        }

        std::string stop_id = tokens[column_index.at("stop_id")];
        int stop_seq = std::stoi(tokens[column_index.at("stop_sequence")]);

        trip_to_stops[trip_id].emplace_back(stop_seq, stop_id);
    } });

    return trip_to_stops;
}

bool Subway::open_and_parse(const std::string &path, const std::vector<std::string> &required_columns, std::function<void(std::ifstream &, const std::unordered_map<std::string, int> &)> callback) const
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "failed to open file: " << path << "\n";
        return false;
    }

    std::string header;
    if (!std::getline(file, header))
    {
        std::cerr << "File is empty or missing header: " << path << "\n";
        return false;
    }

    auto headers = Utils::split(header, ',');
    std::unordered_map<std::string, int> column_index;
    for (int i = 0; i < headers.size(); ++i)
    {
        column_index[headers[i]] = i;
    }

    for (const auto &column : required_columns)
    {
        if (column_index.find(column) == column_index.end())
        {
            std::cerr << "Missing column " << column << " in file " << path << "\n";
            return false;
        }
    }

    callback(file, column_index);
    return true;
}

std::unordered_map<TrainLine, std::vector<Route>> Subway::get_routes() const
{
    return routes;
}
