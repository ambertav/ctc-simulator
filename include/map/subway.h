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

        struct StringHash
        {
            using is_transparent = void;
            [[nodiscard]] size_t operator()(std::string_view sv) const noexcept {
                return std::hash<std::string_view>{}(sv);
            }
        };
        std::unordered_map<std::string, int, StringHash, std::equal_to<>> gtfs_to_id;
        std::unordered_map<TrainLine, std::vector<Route>> routes;

        Subway();
        void load_stations(const std::string &csv);
        void load_connections(const std::string &csv);

    public:
        std::unordered_map<TrainLine, std::vector<Route>> get_routes() const;
    };
}