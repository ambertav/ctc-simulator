#pragma once

#include "core/train.h"
#include "core/station.h"
#include "core/track.h"
#include "core/platform.h"
#include "core/signal.h"

#include "map/base.h"

class Factory
{
private:
    std::unordered_map<int, std::unique_ptr<Train>> trains;
    std::unordered_map<int, std::unique_ptr<Station>> stations;
    std::unordered_map<int, std::unique_ptr<Signal>> signals;
    std::unordered_map<int, std::unique_ptr<Platform>> platforms;
    std::unordered_map<int, std::unique_ptr<Track>> tracks;

public:
    Factory() = default;

    void build_network(int total_trains, const Transit::Map::Path &path);

    std::vector<Train *> get_trains() const;
    std::vector<Station *> get_stations() const;
    std::vector<Signal *> get_signals() const;
    std::vector<Platform *> get_platforms() const;
    std::vector<Track *> get_tracks() const;

    void clear();

private:
    void create_trains(int total, TrainLine train_line);
    std::pair<int, int> create_stations(const Transit::Map::Path &path);
    void create_network(const Transit::Map::Path &path, int start, int end);
};