#pragma once

#include <array>

namespace Yards
{
    constexpr int North = 4000;
    constexpr int South = 4001;

    constexpr std::array<int, 2> ids = {4000, 4001};

    constexpr const char *get_yard_name(int id)
    {
        if (id == North)
        {
            return "North Yard";
        }
        else if (id == South)
        {
            return "South Yard";
        }
        else
        {
            return "Unknown Yard";
        }
    }
}