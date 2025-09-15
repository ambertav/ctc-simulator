/**
 * for details on design, see:
 * docs/system/registry.md
 */

#pragma once

#include <vector>
#include <unordered_map>
#include "enum/transit_types.h"

struct Info
{
    int id;
    int system_code;
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

    const std::vector<int>& get_train_registry(int system_code) const;
    const std::vector<std::pair<int, int>>& get_yard_registry(int system_code) const;

    //[ system (4 bits) | train_line_code (8 bits) | direction_code (12 bits) | instance (8 bits) ]
    int encode(int system_code, int train_line_code, int direction_code, int instance);
    Info decode(int encoded_id) const;

private:
    std::unordered_map<int /* system_code */, std::vector<int /* encoded_id */>> train_registry;
    std::unordered_map<int /* system_code */, std::vector<std::pair<int /* from_encoded_id */, int /* to_encoded_id */>>> yard_registry;

    Registry();

    void build_registry();

    void generate_trains(int system_code, int tl_count, int dir_count);
    void generate_yards(int system_code, int tl_count);
};