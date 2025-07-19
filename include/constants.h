#pragma once

#include <array>
#include <variant>

#include "enums/transit_types.h"

namespace Yards
{
    constexpr int uptown{1000};
    constexpr int downtown{1001};
    constexpr int outbound{2000};
    constexpr int inbound{2001};
    constexpr int westbound{3000};
    constexpr int eastbound{3001};
    constexpr int north{4000};
    constexpr int south{4001};

    constexpr std::array<int, 8> ids = {1000, 1001, 2000, 2001, 3000, 3001, 4000, 4001};

    constexpr std::pair<int, int> get_yard_id_by_direction(const Direction &direction)
    {
        return std::visit([](const auto &dir) -> std::pair<int, int>
                          {
        using T = std::decay_t<decltype(dir)>;
        if constexpr (std::is_same_v<T, SUB::Direction>)
        {
            if (dir == SUB::Direction::DOWNTOWN)
            {
                return {uptown, downtown};
            } 
            else if (dir == SUB::Direction::UPTOWN)
            {
                return {downtown, uptown};
            }
        }
        else if constexpr (std::is_same_v<T, MNR::Direction>)
        {
            if (dir == MNR::Direction::INBOUND)
            {
                return {outbound, inbound};
            } 
            else if (dir == MNR::Direction::OUTBOUND)
            {
                return {inbound, outbound};
            }
        }
        else if constexpr (std::is_same_v<T, LIRR::Direction>)
        {
            if (dir == LIRR::Direction::WESTBOUND)
            {
                return {eastbound, westbound};
            } 
            else if (dir == LIRR::Direction::EASTBOUND)
            {
                return {westbound, eastbound};
            }
        } 
            return {north, south};
        }, direction);
    }

    constexpr const char *get_yard_name(int id)
    {
        switch (id)
        {
        case uptown:
            return "uptown";
        case downtown:
            return "downtown";
        case inbound:
            return "inbound";
        case outbound:
            return "outbound";
        case westbound:
            return "westbound";
        case eastbound:
            return "eastbound";
        case north:
            return "north";
        case south:
            return "south";
        default:
            return "unknown";
        }
    }
}