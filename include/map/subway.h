#pragma once

#include <functional>
#include <string>
#include <vector>
#include <unordered_map>

#include "map/graph.h"
#include "enums/transit_types.h"

namespace Transit::Map
{

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
        std::unordered_map<std::string, int> gtfs_to_id;
        std::unordered_map<TrainLine, std::vector<Route>> routes;

        Subway();
        void load_stations(const std::string &csv);
        void load_connections(const std::string &csv);

        void add_route(const std::string &route_headsign, std::vector<std::string> &gtfs_sequence);

    public:
        std::unordered_map<TrainLine, std::vector<Route>> get_routes() const;
    };
}