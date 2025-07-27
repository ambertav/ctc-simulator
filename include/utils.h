#pragma once

#include <vector>
#include <string_view>
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

    inline std::vector<std::string_view> split(std::string_view sv, char delimiter)
    {
        std::vector<std::string_view> tokens{};
        size_t start{};
        while (true)
        {
            auto pos{sv.find(delimiter, start)};
            if (pos == std::string_view::npos)
            {
                tokens.emplace_back(sv.substr(start));
                break;
            }
            tokens.emplace_back(sv.substr(start, pos - start));
            start = pos + 1;
        }

        return tokens;
    }

    inline std::string_view trim(std::string_view sv)
    {
        const char *begin{sv.data()};
        const char *end{sv.data() + sv.size()};

        while (begin < end && std::isspace(static_cast<unsigned char>(*begin)))
        {
            ++begin;
        }

        while (end > begin && std::isspace(static_cast<unsigned char>(*(end - 1))))
        {
            --end;
        }

        if ((end - begin) >= 2 && *begin == '"' && *(end - 1) == '"')
        {
            ++begin;
            --end;
        }

        return std::string_view(begin, end - begin);
    }

    inline bool open_and_parse(const std::string &file_path, const std::vector<std::string> &required_columns, const std::function<void(std::string_view, const std::unordered_map<std::string_view, int> &, int)> &callback)
    {
        std::ifstream file(file_path, std::ios::binary);
        if (!file)
        {
            std::cerr << "Failed to open: " << file_path << "\n";
            return false;
        }

        file.seekg(0, std::ios::end);
        auto size{file.tellg()};
        file.seekg(0, std::ios::beg);

        std::string buffer(size, '\0');
        file.read(buffer.data(), size);

        size_t header_end{buffer.find('\n')};
        if (header_end == std::string::npos)
        {
            std::cerr << "Missing header in file: " << file_path << "\n";
            return false;
        }

        std::string_view header(buffer.data(), header_end);
        auto headers = split(header, ',');

        std::unordered_map<std::string_view, int> column_index{};
        for (int i = 0; i < headers.size(); ++i)
        {
            column_index[trim(headers[i])] = i;
        }

        for (const auto &column : required_columns)
        {
            if (!column_index.contains(column))
            {
                std::cerr << "Missing column " << column << " in file " << file_path << "\n";
                return false;
            }
        }

        size_t line_start{header_end + 1};
        int line_num{2};

        while (line_start < buffer.size())
        {
            size_t line_end{buffer.find('\n', line_start)};
            if (line_end == std::string::npos)
            {
                line_end = buffer.size();
            }

            std::string_view line(buffer.data() + line_start, line_end - line_start);
            callback(line, column_index, line_num);
            line_start = line_end + 1;
            ++line_num;
        }

        return true;
    }

    inline std::string to_lower_copy(const std::string &str)
    {
        auto result_view = std::views::transform(str, [](unsigned char c)
                                                 { return std::tolower(c); });
        return std::string(result_view.begin(), result_view.end());
    }
}