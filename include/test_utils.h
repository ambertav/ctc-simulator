#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <random>
#include <algorithm>

#include "utils.h"

namespace TestUtils
{
    inline std::vector<int> extract_random_ids(const std::string& file_path, const std::string& column_name, int count)
    {
        std::ifstream file(file_path);
        if (!file.is_open())
        {
            throw std::runtime_error("Could not open station file");
        }

        std::string header{};
        std::getline(file, header);
        auto headers {Utils::split(header, ',')};

        int id_index {-1};
        for (int i = 0; i < headers.size(); ++i)
        {
            if (headers[i] == column_name)
            {
                id_index = i;
                break;
            }
        }

        if (id_index == -1)
        {
            throw std::runtime_error("Missing id column");
        }

        std::vector<int> all_ids{};
        std::string line{};
        while (std::getline(file, line))
        {
            auto tokens {Utils::split(line, ',')};
            if (tokens.size() > id_index)
            {
                all_ids.push_back(Utils::string_view_to_numeric<int>(tokens[id_index]));
            }
        }

        std::shuffle(all_ids.begin(), all_ids.end(), std::mt19937{std::random_device{}()});
        all_ids.resize(std::min((int)all_ids.size(), count));

        return all_ids;
    }
}