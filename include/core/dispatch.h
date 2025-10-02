/**
 * for details on design, see:
 * docs/dispatch.md
 */

#pragma once

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <variant>
#include <optional>
#include <map>
#include <memory>

#include "enum/event_type.h"
#include "core/train.h"
#include "core/track.h"
#include "core/platform.h"
#include "core/station.h"
#include "system/logger.h"

class CentralControl;

struct Event
{
    int tick;
    int train_id;
    int station_id;
    Direction direction;
    EventType type;

    Event(int t, int t_id, int s_id, Direction dir, EventType ty) : tick(t), train_id(t_id), station_id(s_id), direction(dir), type(ty) {}

    bool operator<(const Event &other) const
    {
        return tick > other.tick;
    }
};

struct EventQueues
{
    std::multimap<int /* tick */, Event> arrivals;
    std::multimap<int /* tick */, Event> departures;
};

class Dispatch
{
private:
    std::unordered_map<int, Station *> stations;
    std::vector<Station *> yards;
    std::vector<Train *> trains;
    std::unordered_set<Signal *> failed_signals;
    std::vector<std::pair<Train *, Track *>> authorized;
    std::unordered_map<int, EventQueues> schedule;

    CentralControl *central_control;
    Logger *logger;
    TrainLine train_line;

public:
    Dispatch(CentralControl *cc, TrainLine tl, const std::vector<Station *> &st, const std::vector<Train *> &tn, Logger *log);

    TrainLine get_train_line() const;
    const std::unordered_map<int, Station *> &get_stations() const;
    const std::vector<Train *> &get_trains() const;
    const std::vector<std::pair<Train *, Track *>> &get_authorizations() const;
    const std::unordered_map<int, EventQueues> &get_station_schedules() const;

    void load_schedule();
    void authorize(int tick);
    void execute(int tick);

private:
    void handle_signal_state(int tick, Signal *signal, bool set_red = false);
    bool needs_yellow_signal(Track* track);
    std::optional<Event> process_event(int tick, std::multimap<int, Event> &queue, Train *train);

    void handle_spawns(int tick);
    void spawn_train(int tick, const Event &event);
    void despawn_train(int tick, const Event &event, Train *train, int yard_id);

    int calculate_switch_priority(int tick, Train *train);
    std::pair<bool, int> randomize_delay(double probability);
};