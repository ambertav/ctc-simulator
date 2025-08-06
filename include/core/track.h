/**
 * for details on design, see:
 * docs/track.md
 */

#pragma once

#include <vector>
#include <unordered_set>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <atomic>

#include "enums/transit_types.h"

class Train;
class Platform;
class Signal;
class Switch;

class Track
{
private:
    const int id;
    int duration;
    std::atomic<bool> occupied;
    std::atomic<Train *> current_train;
    Signal *const signal;
    std::unordered_set<TrainLine> train_lines;

    mutable std::shared_mutex mutex;
    std::vector<Track *> next_tracks;
    std::vector<Track *> prev_tracks;

    std::vector<Switch *> outbound_switches;
    std::vector<Switch *> inbound_switches;

public:
    Track(int i, Signal *s, int d = 1, std::unordered_set<TrainLine> lines = {});

    int get_id() const;
    int get_duration() const;
    Signal *get_signal() const;
    const Train *get_occupying_train() const;

    const std::vector<Track *> &get_next_tracks() const;
    const std::vector<Track *> &get_prev_tracks() const;
    const std::vector<Switch *> &get_outbound_switches() const;
    const std::vector<Switch *> &get_inbound_switches() const;

    bool is_occupied() const;
    virtual bool supports_train_line(TrainLine line) const;
    virtual bool is_platform() const;

    bool try_entry(Train *train);
    void release_train();

    void add_train_line(TrainLine line);
    void remove_train_line(TrainLine line);

    void add_next_track(Track *next);
    void add_prev_track(Track *prev);
    void remove_next_track(Track *next);
    void remove_prev_track(Track *next);

    void add_outbound_switch(Switch *sw);
    void add_inbound_switch(Switch *sw);
    void remove_outbound_switch(Switch *sw);
    void remove_inbound_switch(Switch *sw);
};