#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <thread>
#include <chrono>

#include "utils.h"
#include "map/graph.h"

namespace TestUtils
{
    inline bool coin_flip(double p = 0.5)
    {
        std::bernoulli_distribution dist(p);
        return dist(Utils::rng());
    }

    inline std::vector<int> extract_random_ids(const std::string &file_path, const std::string &column_name, size_t count)
    {
        std::ifstream file(file_path);
        if (!file.is_open())
        {
            throw std::runtime_error("Could not open station file");
        }

        std::string header{};
        std::getline(file, header);
        auto headers{Utils::split(header, ',')};

        int id_index{-1};
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
            auto tokens{Utils::split(line, ',')};
            if (tokens.size() > id_index)
            {
                all_ids.push_back(Utils::string_view_to_numeric<int>(tokens[id_index]));
            }
        }

        std::shuffle(all_ids.begin(), all_ids.end(), Utils::rng());
        all_ids.resize(std::min(all_ids.size(), count));

        return all_ids;
    }

    inline std::vector<int> extract_random_ids(Transit::Map::Graph &graph, size_t count)
    {
        const auto &adj_list{graph.get_adjacency_list()};
        std::vector<int> all_ids{};
        all_ids.reserve(adj_list.size());

        for (const auto &[id, _] : adj_list)
        {
            all_ids.push_back(id);
        }

        std::shuffle(all_ids.begin(), all_ids.end(), Utils::rng());
        all_ids.resize(std::min(all_ids.size(), count));

        return all_ids;
    }

    template <typename W, typename R>
    void run_concurrency_write_read(W writer, R reader, int writer_count = 50, int reader_count = 10, int duration_ms = 500)
    {
        std::vector<std::thread> writer_threads{};
        std::vector<std::thread> reader_threads{};

        writer_threads.reserve(writer_count);
        reader_threads.reserve(reader_count);

        std::atomic<bool> done{false};

        for (int i = 0; i < writer_count; ++i)
        {
            writer_threads.emplace_back([&, i]()
                                        {
            try
            {
                writer(i, done);
            } catch (const std::exception& e)
            {
                FAIL() << "Writer thread " << i << " threw exception: " << e.what(); 
            } });
        }

        for (int i = 0; i < reader_count; ++i)
        {
            reader_threads.emplace_back([&, i]()
                                        {
            try
            {
                reader(i, done);
            } catch (const std::exception& e)
            {
                FAIL() << "Reader thread " << i << " threw exception: " << e.what(); 
            } });
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));
        done.store(true, std::memory_order_release);

        for (auto &t : writer_threads)
        {
            t.join();
        }

        for (auto &t : reader_threads)
        {
            t.join();
        }
    }
}