#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "enums/transit_types.h"
#include "map/graph.h"

namespace Transit::Map
{
    class MetroNorth : public Graph
    {
    public:
        static MetroNorth &get_instance()
        {
            static MetroNorth instance;
            return instance;
        }

        MetroNorth(const MetroNorth &) = delete;
        MetroNorth &operator=(const MetroNorth &) = delete;

    private:
        std::unordered_map<TrainLine, std::vector<Route>> routes;

        MetroNorth();
        void load_stations(const std::string &csv);
        void load_connections(const std::string &csv);

        std::vector<int> merge_segments(const std::vector<std::vector<int>> &segments);
        std::vector<int> k_way_merge(const std::vector<std::vector<int>> &segments, const std::unordered_map<int, std::unordered_set<int>> &precedence);
    };
}