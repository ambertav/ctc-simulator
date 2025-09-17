/**
 * for details on design, see:
 * docs/system/factory.md
 */

#pragma once

#include <memory>

#include "core/train.h"
#include "core/station.h"
#include "core/track.h"
#include "core/platform.h"
#include "core/signal.h"
#include "core/switch.h"

#include "system/registry.h"
#include "map/graph.h"

class Factory
{
private:
    int next_signal_id{1};
    int next_track_id{1};
    int next_switch_id{1};

    std::unordered_map<int, std::unique_ptr<Train>> trains;
    std::unordered_map<int, std::unique_ptr<Station>> stations;
    std::unordered_map<int, std::unique_ptr<Signal>> signals;
    std::unordered_map<int, std::unique_ptr<Platform>> platforms;
    std::unordered_map<int, std::unique_ptr<Track>> tracks;
    std::unordered_map<int, std::unique_ptr<Switch>> switches;

public:
    Factory() = default;

    void build_network(const Transit::Map::Graph &graph, const Registry &registry, Constants::System system_code);

    std::vector<Train *> get_trains() const;
    std::vector<Train *> get_trains(TrainLine train_line) const;
    std::vector<Station *> get_stations() const;
    std::vector<Station *> get_stations(TrainLine train_line) const;
    std::vector<Signal *> get_signals() const;
    std::vector<Platform *> get_platforms() const;
    std::vector<Track *> get_tracks() const;
    std::vector<Switch *> get_switches() const;

private:
    int generate_signal_id();
    int generate_track_id();
    int generate_switch_id();

    void create_trains(const Registry &registry, Constants::System system_code);
    void create_stations(const Transit::Map::Graph &graph, const Registry &registry, Constants::System system_code);
    void create_track(Station *from, Station *to, TrainLine train_line, Direction direction, int duration);
    void create_switch(Platform *from, Platform *to, Track *track);
};