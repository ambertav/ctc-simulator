#pragma once

#include <functional>

#include "map/graph.h"
#include "enums/transit_types.hpp"

namespace Transit::Map
{
    struct Route
    {
        std::string headsign;
        std::vector<std::string> sequence;

        Route(const std::string &h, std::vector<std::string> &s) : headsign(h), sequence(s) {}
    };

    class Subway : public Graph
    {
    public:
        static Subway &get_instance()
        {
            static Subway instance;
            return instance;
        }

        Subway(const Subway &) = delete;
        Subway &operator=(const Subway &) = delete;

    private:
        std::unordered_map<std::string, std::string> gtfs_to_id;
        std::unordered_map<TrainLine, std::vector<Route>> routes;

        Subway();
        void load_stations(const std::string &csv);
        void load_connections(const std::string &trips, const std::string &stop_times);

        void add_route(const std::string &route_headsign, std::vector<std::string> &gtfs_sequence);

        std::unordered_map<std::string, std::string> extract_headsign_to_trip(const std::string &trips);
        std::unordered_map<std::string, std::vector<std::pair<int, std::string>>> extract_trip_to_stops(const std::string &stop_times, const std::unordered_set<std::string> &valid_trip_ids);

        bool open_and_parse(const std::string &path, const std::vector<std::string> &required_columns, std::function<void(std::ifstream &, const std::unordered_map<std::string, int> &)> callback) const;

    public:
        std::unordered_map<TrainLine, std::vector<Route>> get_routes() const;
    };
}