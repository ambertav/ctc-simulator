/**
 * for details on design, see:
 * docs/system/registry.md
 */

#pragma once

#include <vector>
#include <unordered_map>
#include <optional>
#include <functional>

#include "constants/constants.h"
#include "enum/transit_types.h"
#include "map/graph.h"

struct Info
{
    int id;
    Constants::System system_code;
    TrainLine train_line;
    Direction direction;
    int instance;
};

class Registry
{
public:
    static Registry &get_instance()
    {
        static Registry instance;
        return instance;
    }

    Registry(const Registry &) = delete;
    Registry &operator=(const Registry &) = delete;

    const std::vector<int>& get_train_registry(Constants::System system_code) const;
    const std::vector<std::pair<int, int>>& get_yard_registry(Constants::System system_code) const;

    //[ system (4 bits) | train_line_code (8 bits) | direction_code (12 bits) | instance (8 bits) ]
    int encode(Constants::System system_code, int train_line_code, int direction_code, int instance);
    Info decode(int encoded_id) const;

    void register_route(int train_id, const Transit::Map::Route& route);
    std::optional<std::reference_wrapper<const Transit::Map::Route>> get_registered_route(int train_id) const;

private:
    std::unordered_map<Constants::System, std::vector<int /* encoded_id */>> train_registry;
    std::unordered_map<Constants::System, std::vector<std::pair<int /* from_encoded_id */, int /* to_encoded_id */>>> yard_registry;
    std::unordered_map<int /* train_id */, const Transit::Map::Route&> route_registry;

    Registry();

    void build_registry();

    void generate_trains(Constants::System system_code, int tl_count, int dir_count);
    void generate_yards(Constants::System system_code, int tl_count);
};