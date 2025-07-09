#pragma once
#include <vector>
#include <string>
#include <sstream>

namespace Utils {
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
}