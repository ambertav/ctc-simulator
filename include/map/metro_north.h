/**
 * for details on design, see:
 * docs/map/metro_north.md
 */

#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "constants/constants.h"
#include "enum/transit_types.h"
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
        double weight_scale_factor {Constants::METRO_NORTH_SCALE_FACTOR};

        MetroNorth();
        void load_stations(const std::string &csv);
        void load_connections(const std::string &csv);

        std::vector<int> merge_segments(const std::vector<std::vector<int>> &segments);
        std::vector<int> k_way_merge(const std::vector<std::vector<int>> &segments, const std::unordered_map<int, std::unordered_set<int>> &precedence);

        std::vector<int> handle_branches(std::string_view branch_name, const std::vector<std::vector<int>> &segments);
        int get_branch_point(std::string_view branch_name) const;
        std::string_view get_branch_point_name(std::string_view branch_name) const;
    };
}