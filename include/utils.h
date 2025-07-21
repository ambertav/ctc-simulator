#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <ranges>

namespace Utils
{

    constexpr double EARTH_RADIUS_KM{6371.0};
    constexpr double EARTH_RADIUS_MILES{3959.0};
    constexpr double DEG_TO_RAD{M_PI / 180.0};

    constexpr double TRANSFER_EPSILON{0.001};

    inline std::string trim(const std::string &str)
{
    const size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    const size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}


    inline std::vector<std::string> split(const std::string &s, char delimiter)
    {
        std::vector<std::string> tokens;
        std::stringstream ss(s);
        std::string token;

        while (std::getline(ss, token, delimiter))
        {
            tokens.push_back(trim(token));
        }

        if (!s.empty() && s.back() == delimiter)
        {
            tokens.push_back("");
        }

        return tokens;
    }

    inline bool open_and_parse(const std::string &file_path, const std::vector<std::string> &required_columns, std::function<void(std::ifstream &, const std::unordered_map<std::string, int> &)> callback)
    {
        std::ifstream file(file_path);
        if (!file.is_open())
        {
            std::cerr << "failed to open file: " << file_path << "\n";
            return false;
        }

        std::string header;
        if (!std::getline(file, header))
        {
            std::cerr << "File is empty or missing header: " << file_path << "\n";
            return false;
        }

        auto headers = Utils::split(header, ',');
        std::unordered_map<std::string, int> column_index;
        for (int i = 0; i < headers.size(); ++i)
        {
            column_index[headers[i]] = i;
        }

        for (const auto &column : required_columns)
        {
            if (column_index.find(column) == column_index.end())
            {
                std::cerr << "Missing column " << column << " in file " << file_path << "\n";
                return false;
            }
        }

        callback(file, column_index);
        return true;
    }

    inline std::string to_lower_copy(const std::string &str)
    {
        auto result_view = std::views::transform(str, [](unsigned char c)
                                                 { return std::tolower(c); });
        return std::string(result_view.begin(), result_view.end());
    }
}