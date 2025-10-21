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

    inline constexpr int DEFAULT_TRAINS_PER_LINE{10};
    inline constexpr int DEFAULT_DWELL_TIME{2};
    inline constexpr int DEFAULT_TRAVEL_TIME{2};
    inline constexpr int DEFAULT_YARD_HEADWAY{6};
    inline constexpr int MAX_TRACK_DURATION{4};

    inline constexpr double PLATFORM_DELAY_PROBABILITY{0.3};
    inline constexpr double SIGNAL_FAILURE_PROBABILITY{0.05};
    inline constexpr double SWITCH_FAILURE_PROBABILITY{0.02};
    inline constexpr int MAX_DELAY{4};

    inline constexpr double SUBWAY_SCALE_FACTOR{0.5};
    inline constexpr double METRO_NORTH_SCALE_FACTOR{1.8};
    inline constexpr double LIRR_SCALE_FACTOR{1.4};

    enum class System
    {
        SUBWAY = 1,
        METRO_NORTH = 2,
        LIRR = 3
    };

    inline constexpr std::array<std::pair<const char *, System>, 3> SYSTEMS{{{"subway", System::SUBWAY},
                                                                             {"metro_north", System::METRO_NORTH},
                                                                             {"lirr", System::LIRR}}};

    inline constexpr std::array<std::pair<System, std::array<Direction, 2>>, 3> SYSTEM_DIRECTION_ORDER{{{System::SUBWAY, {SUB::Direction::UPTOWN, SUB::Direction::DOWNTOWN}},
                                                                                                        {System::METRO_NORTH, {MNR::Direction::INBOUND, MNR::Direction::OUTBOUND}},
                                                                                                        {System::LIRR, {LIRR::Direction::WESTBOUND, LIRR::Direction::EASTBOUND}}}};

    inline std::array<Direction, 2> get_directions_by_system_code(System code)
    {
        auto it{std::find_if(SYSTEM_DIRECTION_ORDER.begin(), SYSTEM_DIRECTION_ORDER.end(), [&](const auto &entry)
                             { return entry.first == code; })};

        if (it == SYSTEM_DIRECTION_ORDER.end())
        {
            throw std::runtime_error("Unknown system encountered when getting directions");
        }

        return it->second;
    }
}