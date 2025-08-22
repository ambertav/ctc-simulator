#pragma once

#include <array>

#include "config.h"
#include "enum/transit_types.h"

namespace Constants
{
    inline constexpr double EARTH_RADIUS_KM{6371.0};
    inline constexpr double EARTH_RADIUS_MILES{3959.0};
    inline constexpr double DEG_TO_RAD{M_PI / 180.0};

    inline constexpr double TRANSFER_EPSILON{0.001};

    inline constexpr int TRAINS_PER_LINE{10};
    inline constexpr int STATION_DWELL_TIME{2};
    inline constexpr int DEFAULT_TRAVEL_TIME{2};
    inline constexpr int YARD_DEPARTURE_GAP{2};

    inline constexpr double SUBWAY_SCALE_FACTOR{0.5};
    inline constexpr double METRO_NORTH_SCALE_FACTOR{1.8};
    inline constexpr double LIRR_SCALE_FACTOR{1.4};

    enum class System
    {
        SUBWAY = 1,
        METRO_NORTH = 2,
        LIRR = 3
    };

    inline constexpr std::array<std::pair<const char *, System>, 3> SYSTEMS{{
        {"subway", System::SUBWAY},
        {"metro_north", System::METRO_NORTH},
        {"lirr", System::LIRR}}};

    inline constexpr std::array<std::pair<System, std::array<int, 2>>, 3> SYSTEM_DIRECTION_ORDER{{
        {System::SUBWAY, {static_cast<int>(SUB::Direction::UPTOWN), static_cast<int>(SUB::Direction::DOWNTOWN)}},
        {System::METRO_NORTH, {static_cast<int>(MNR::Direction::INBOUND), static_cast<int>(MNR::Direction::OUTBOUND)}},
        {System::LIRR, {static_cast<int>(LIRR::Direction::WESTBOUND), static_cast<int>(LIRR::Direction::EASTBOUND)}}}};

}