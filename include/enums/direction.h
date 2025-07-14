#pragma once

#include "map/base.h"
#include <iostream>

enum class Direction
{
    UPTOWN,
    DOWNTOWN,
    UNKNOWN
};

inline Direction direction_from_string(const std::string &str)
{
    static const std::unordered_map<std::string, Direction> direction_mapping{
        {"0", Direction::UPTOWN},
        {"1", Direction::DOWNTOWN},
    };

    auto it = direction_mapping.find(str);
    if (it != direction_mapping.end())
    {
        return it->second;
    }
    else
    {
        return Direction::UNKNOWN;
    }
}

inline Direction infer_direction(const Transit::Map::Node *start, const Transit::Map::Node *end)
{
    if (!start || !end)
    {
        throw std::invalid_argument("Start or end node is null");
    }

    return start->latitude > end->latitude ? Direction::DOWNTOWN : Direction::UPTOWN;
}

inline std::ostream &operator<<(std::ostream &os, Direction direction)
{
    switch (direction)
    {
    case Direction::UPTOWN:
        return os << "uptown";
    case Direction::DOWNTOWN:
        return os << "downtown";
    case Direction::UNKNOWN:
        return os << "unknown";
    }
}