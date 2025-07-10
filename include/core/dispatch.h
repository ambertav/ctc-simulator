#pragma once

#include <vector>
#include <unordered_map>
#include <queue>

#include "core/train.h"
#include "core/track.h"
#include "core/platform.h"
#include "core/signal.h"
#include "core/station.h"
#include "systems/logger.h"

enum class EventType
{
    ARRIVAL,
    DEPARTURE,
    SIGNAL
};

struct Event
{
    int tick;
    int train_id;
    int station_id;
    Direction direction;
    EventType type;

    Event(int t, int t_id, int s_id, Direction dir, EventType ty) : tick(t), train_id(t_id), station_id(s_id), direction(dir), type(ty) {}

    // for min-heap, earliest on top
    bool operator<(const Event &other) const
    {
        return tick > other.tick;
    }
};

struct EventQueues
{
    std::priority_queue<Event> arrivals;
    std::priority_queue<Event> departures;
};

class Dispatch
{
private:
    std::unordered_map<int, Station *> stations;
    std::vector<Train *> trains;
    std::vector<Track *> segments;
    std::vector<Signal *> signals;

    std::unordered_map<int, EventQueues> schedule;

    Logger &logger;

public:
    Dispatch(const std::vector<Station *> &st, const std::vector<Train *> &tn, const std::vector<Track *> &tk, const std::vector<Platform *> &p, const std::vector<Signal *> &si, Logger &log);

    const std::unordered_map<int, Station *> &get_stations() const;
    const std::vector<Train *> &get_trains() const;
    const std::vector<Signal *> &get_signals() const;
    const std::vector<Track *> &get_segments() const;

    const std::unordered_map<int, EventQueues> &get_schedule() const;

    void load_schedule(const std::string &csv_file);
    void update(int tick);

private:
    void handle_signals(int tick);
    void handle_spawns(int tick);
    void spawn_train(int tick, Event event);
    void despawn_train(Train *train, int yard_id);
};