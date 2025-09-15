#pragma once

#include <vector>
#include <string_view>
#include <string>
#include <fstream>
#include <sstream>
#include <ranges>
#include <charconv>
#include <random>
#include <iostream>

#include "system/registry.h"

namespace Utils
{

    inline std::string_view trim(std::string_view sv)
    {
        const char *begin = sv.data();
        const char *end = sv.data() + sv.size();

        while (begin < end && (std::isspace(static_cast<unsigned char>(*begin)) || *begin == '\r' || *begin == '\n'))
            ++begin;

        while (end > begin && (std::isspace(static_cast<unsigned char>(*(end - 1))) || *(end - 1) == '\r' || *(end - 1) == '\n'))
            --end;

        if ((end - begin) >= 2 && *begin == '"' && *(end - 1) == '"')
        {
            ++begin;
            --end;
        }

        return std::string_view(begin, end - begin);
    }

    inline std::vector<std::string_view> split(std::string_view sv, char delimiter)
    {
        std::vector<std::string_view> tokens{};
        size_t start{};
        while (true)
        {
            auto pos{sv.find(delimiter, start)};
            std::string_view token{};
            if (pos == std::string_view::npos)
            {
                token = sv.substr(start);
                token = trim(token);
                tokens.emplace_back(token);
                break;
            }
            tokens.emplace_back(sv.substr(start, pos - start));
            start = pos + 1;
        }

        return tokens;
    }

    inline bool open_and_parse(const std::string &file_path, const std::vector<std::string_view> &required_columns, const std::function<void(std::string_view, const std::unordered_map<std::string_view, int> &, int)> &callback)
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

    inline std::unordered_map<std::string_view, std::string_view> from_tokens(const std::vector<std::string_view> &tokens, const std::unordered_map<std::string_view, int> &column_index)
    {
        std::unordered_map<std::string_view, std::string_view> row{};
        for (auto &[column, index] : column_index)
        {
            if (index < tokens.size())
            {
                row[column] = Utils::trim(tokens[index]);
            }
        }

        return row;
    }

    inline void log_malformed_line(const std::string &system_name, const std::string &file_type, int line_num, std::string_view line)
    {
        std::cerr << "In " << system_name << " " << file_type << ", malformed line at " << line_num << ": " << line << "\n";
    }

    inline void log_file_open_error(const std::string &system_name, const std::string &file_type, const std::string &file_path)
    {
        std::cerr << "Could not open " << system_name << " " << file_type << "output, file path: " << file_path << "\n";
    }

    inline std::string to_lower_copy(const std::string &str)
    {
        auto result_view = std::views::transform(str, [](unsigned char c)
                                                 { return std::tolower(c); });
        return std::string(result_view.begin(), result_view.end());
    }

    template <typename T>
    inline T string_view_to_numeric(std::string_view sv)
    {
        if constexpr (std::is_floating_point_v<T>)
        {

            std::string temp(sv);
            if constexpr (std::is_same_v<T, float>)
            {
                return std::stof(temp);
            }
            else if constexpr (std::is_same_v<T, double>)
            {
                return std::stod(temp);
            }
        }
        else
        {
            T result{};
            auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), result);

            if (ec != std::errc{})
            {
                if (ec == std::errc::invalid_argument)
                {
                    throw std::invalid_argument("Invalid numeric format");
                }
                else if (ec == std::errc::result_out_of_range)
                {
                    throw std::out_of_range("Number out of range");
                }
            }

            return result;
        }
    }

    inline std::string generate_yard_name(const Info &yard)
    {
        return direction_to_string(yard.direction) + " " + trainline_to_string(yard.train_line) + " yard";
    }

    inline std::mt19937 &rng()
    {
        thread_local static std::mt19937 gen{std::random_device{}()};
        return gen;
    }

    inline std::size_t random_index(std::size_t size)
    {
        if (size == 0)
        {
            throw std::invalid_argument("TestUtil::random_index called with size 0");
        }

        std::uniform_int_distribution<std::size_t> dist(0, size - 1);
        return dist(rng());
    }
}