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
    inline constexpr int YARD_DEPARTURE_GAP{2};

    inline constexpr std::array<std::pair<const char *, int>, 3> SYSTEMS{{
        {"subway", 1},
        {"metro north", 2},
        {"long island", 3}}};
}