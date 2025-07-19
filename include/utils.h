#pragma once
#include <vector>
#include <string>
#include <sstream>
#include <ranges>

namespace Utils
{

    constexpr double EARTH_RADIUS_KM{6371.0};
    constexpr double EARTH_RADIUS_MILES{3959.0};
    constexpr double DEG_TO_RAD{M_PI / 180.0};

    constexpr double TRANSFER_EPSILON{0.001};

    inline std::vector<std::string> split(const std::string &s, char delimiter)
    {
        std::vector<std::string> tokens;
        std::stringstream ss(s);
        std::string token;

        while (std::getline(ss, token, delimiter))
        {
            tokens.push_back(token);
        }

        if (!s.empty() && s.back() == delimiter)
        {
            tokens.push_back("");
        }

        return tokens;
    }

    inline std::string to_lower_copy(const std::string &str)
    {
        auto result_view = std::views::transform(str, [](unsigned char c)
                                                 { return std::tolower(c); });
        return std::string(result_view.begin(), result_view.end());
    }
}