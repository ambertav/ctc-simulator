#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "enum/transit_types.h"
#include "map/graph.h"

namespace Transit::Map
{
    class LongIslandRailroad : public Graph
    {
    public:
        static LongIslandRailroad &get_instance()
        {
            static LongIslandRailroad instance;
            return instance;
        }

        LongIslandRailroad(const LongIslandRailroad &) = delete;
        LongIslandRailroad &operator=(const LongIslandRailroad &) = delete;

    private:
        LongIslandRailroad();
        void load_stations(const std::string &csv);
        void load_connections(const std::string &csv);

        std::vector<int> merge_segments(const std::vector<std::vector<int>> &segments);
        std::vector<int> k_way_merge(const std::vector<std::vector<int>> &segments, const std::unordered_map<int, std::unordered_set<int>> &precedence);
    };
}