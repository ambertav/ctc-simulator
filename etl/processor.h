#pragma once

#include <algorithm>
#include <ranges>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "config.h"
#include "utils.h"

namespace etl
{

    struct Job
    {
        std::string name;
        std::string station_file;
        std::string trips_file;
        std::string stop_times_file;
        std::string output_station_file;
        std::string output_routes_file;
    };

    inline void handle_subway_stations(const Job &job)
    {
        const std::vector<std::string> needed_columns{
            "GTFS Stop ID", "Complex ID", "Stop Name",
            "Daytime Routes", "GTFS Latitude", "GTFS Longitude"};

        std::ofstream out(job.output_station_file);
        if (!out.is_open())
        {
            std::cerr << "Error: could not open stations output: "
                      << job.output_station_file << "\n";
            return;
        }

        out << "complex_id,gtfs_id,stop_name,train_lines,latitude,longitude\n";

        Utils::open_and_parse(

            job.station_file, needed_columns,
            [&](std::ifstream &file,
                const std::unordered_map<std::string, int> &column_index)
            {
                std::string line;
                int line_num{};
                while (std::getline(file, line))
                {
                    ++line_num;
                    auto tokens = Utils::split(line, ',');
                    if (tokens.size() < column_index.size())
                    {
                        std::cerr << "In subway stations, malformed line " << line_num
                                  << ": " << line << "\n";
                        continue;
                    }

                    std::string gtfs{tokens[column_index.at("GTFS Stop ID")]};
                    std::string complex{tokens[column_index.at("Complex ID")]};
                    std::string stop_name{tokens[column_index.at("Stop Name")]};
                    std::string daytime_routes{tokens[column_index.at("Daytime Routes")]};
                    std::string latitude{tokens[column_index.at("GTFS Latitude")]};
                    std::string longitude{tokens[column_index.at("GTFS Longitude")]};

                    out << complex << "," << gtfs << "," << stop_name << ","
                        << daytime_routes << "," << latitude << "," << longitude << "\n";
                }
                out.flush();
                out.close();
            });
    }

    inline void handle_subway_routes(const Job &job)
    {
        auto extract_headsign_to_trip = [](const std::string &trips_file)
        {
            std::unordered_map<std::string, std::string> headsign_to_trip{};
            const std::vector<std::string> needed_columns{
                "route_id", "trip_id", "service_id", "trip_headsign"};

            Utils::open_and_parse(
                trips_file, needed_columns,
                [&](std::ifstream &file,
                    const std::unordered_map<std::string, int> &column_index)
                {
                    std::unordered_set<std::string> used_pairs;

                    std::string line;
                    int line_num{};

                    while (std::getline(file, line))
                    {
                        ++line_num;
                        auto tokens = Utils::split(line, ',');

                        if (tokens.size() < column_index.size())
                        {
                            std::cerr << "In subway trips.txt, malformed line " << line_num
                                      << ": " << line << "\n";
                            continue;
                        }

                        std::string route_id{tokens[column_index.at("route_id")]};
                        std::string trip_id{tokens[column_index.at("trip_id")]};
                        std::string service_id{tokens[column_index.at("service_id")]};
                        std::string trip_headsign{tokens[column_index.at("trip_headsign")]};

                        if (service_id != "Saturday")
                        {
                            continue;
                        }

                        if (!route_id.empty() && route_id.back() == 'X')
                        {
                            continue;
                        }

                        std::string unique_key{route_id + std::string("|") + trip_headsign};
                        if (used_pairs.find(unique_key) == used_pairs.end())
                        {
                            headsign_to_trip[unique_key] = trip_id;
                            used_pairs.insert(unique_key);
                        }
                    }
                });

            return headsign_to_trip;
        };

        auto extract_trip_to_stops = [](const std::string &stop_times_file,
                                        const std::unordered_set<std::string>
                                            &valid_trip_ids)
        {
            std::unordered_map<std::string, std::vector<std::pair<int, std::string>>>
                trip_to_stops;
            const std::vector<std::string> needed_columns{"trip_id", "stop_id",
                                                          "stop_sequence"};

            Utils::open_and_parse(
                stop_times_file, needed_columns,
                [&](std::ifstream &file,
                    const std::unordered_map<std::string, int> &column_index)
                {
                    std::string line;
                    int line_num{};

                    while (std::getline(file, line))
                    {
                        ++line_num;

                        auto tokens = Utils::split(line, ',');
                        if (tokens.size() < column_index.size())
                        {
                            std::cerr << "In stop_times.txt, malformed line " << line_num
                                      << ": " << line << "\n";
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
                    }
                });

            return trip_to_stops;
        };

        auto headsign_to_trip = extract_headsign_to_trip(job.trips_file);

        std::unordered_set<std::string> valid_trip_ids;
        for (const auto &[_, trip_id] : headsign_to_trip)
        {
            valid_trip_ids.insert(trip_id);
        }

        auto trip_to_stops =
            extract_trip_to_stops(job.stop_times_file, valid_trip_ids);

        std::ofstream out(job.output_routes_file);
        if (!out.is_open())
        {
            std::cerr << "Error: coultd not open routes output: "
                      << job.output_routes_file << "\n";
            return;
        }

        out << "route_id,headsign,ordered_stops\n";

        for (const auto &[route_headsign, trip_id] : headsign_to_trip)
        {
            if (!trip_to_stops.count(trip_id))
            {
                continue;
            }

            std::vector<std::pair<int, std::string>> sorted_stops = trip_to_stops[trip_id];
            std::ranges::sort(sorted_stops);

            auto transformed_sequence_view =
                sorted_stops | std::views::transform([](const auto &pair)
                                                     {
          std::string cleaned_gtfs{pair.second};
          if (!cleaned_gtfs.empty()) {
            cleaned_gtfs = cleaned_gtfs.substr(0, cleaned_gtfs.size() - 1);
          }

          return cleaned_gtfs; });

            std::vector<std::string> sequence{transformed_sequence_view.begin(),
                                              transformed_sequence_view.end()};

            auto tokens = Utils::split(route_headsign, '|');
            if (tokens.size() < 2)
            {
                continue;
            }

            std::string route_id{tokens[0]};
            std::string headsign{tokens[1]};

            std::string ordered_stops;
            for (int i = 0; i < sequence.size(); ++i)
            {
                if (i > 0)
                {
                    ordered_stops += " ";
                }

                ordered_stops += sequence[i];
            }

            out << route_id << "," << headsign << "," << ordered_stops << "\n";
        }

        out.flush();
        out.close();
    }

    inline void handle_mnr_stations(const Job &job) {}
    inline void handle_mnr_routes(const Job &job) {}

    inline void process_stations(const Job &job)
    {
        if (job.name == "subway")
        {
            handle_subway_stations(job);
        }
        else if (job.name == "mnr")
        {
            handle_mnr_stations(job);
        }
        else
        {
            throw std::invalid_argument("Invalid job, could not process stations");
        }
    }

    inline void process_routes(const Job &job)
    {
        if (job.name == "subway")
        {
            handle_subway_routes(job);
        }
        else if (job.name == "mnr")
        {
            handle_mnr_routes(job);
        }
        else
        {
            throw std::invalid_argument("Invalid job, could not process routes");
        }
    }
}