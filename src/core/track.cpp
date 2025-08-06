/**
 * for details on design, see:
 * docs/track.md
 */

#include <ranges>
#include <algorithm>

#include "core/signal.h"
#include "core/track.h"

Track::Track(int i, Signal *s, int d, std::unordered_set<TrainLine> lines)
    : id(i), duration(std::max(1, d)), occupied(false), signal(s), current_train(nullptr), train_lines(std::move(lines)) {}

int Track::get_id() const
{
    return id;
}

int Track::get_duration() const
{
    return duration;
}

Signal *Track::get_signal() const
{
    return signal;
}

const Train *Track::get_occupying_train() const
{
    return current_train.load(std::memory_order_acquire);
}

const std::vector<Track *> &Track::get_next_tracks() const
{
    std::shared_lock lock(mutex);
    return next_tracks;
}

const std::vector<Track *> &Track::get_prev_tracks() const
{
    std::shared_lock lock(mutex);
    return prev_tracks;
}

const std::vector<Switch *> &Track::get_outbound_switches() const
{
    std::shared_lock lock(mutex);
    return outbound_switches;
}

const std::vector<Switch *> &Track::get_inbound_switches() const
{
    std::shared_lock lock(mutex);
    return inbound_switches;
}

bool Track::is_occupied() const
{
    return occupied.load(std::memory_order_acquire);
}

bool Track::supports_train_line(TrainLine line) const
{
    return train_lines.contains(line);
}

bool Track::is_platform() const
{
    return false;
}

bool Track::try_entry(Train *train)
{
    if (train == nullptr || signal == nullptr)
    {
        return false;
    }

    if (!signal->is_green())
    {
        return false;
    }

    bool expected{false};
    if (!occupied.compare_exchange_strong(expected, true, std::memory_order_acq_rel))
    {
        return false;
    }

    current_train.store(train, std::memory_order_release);
    return true;
}

void Track::release_train()
{
    current_train.store(nullptr, std::memory_order_release);
    occupied.store(false, std::memory_order_release);
}

void Track::add_train_line(TrainLine line)
{
    std::unique_lock lock(mutex);
    train_lines.insert(line);
}

void Track::remove_train_line(TrainLine line)
{
    std::unique_lock lock(mutex);
    train_lines.erase(line);
}

void Track::add_next_track(Track *next)
{
    if (next == nullptr)
    {
        return;
    }

    std::unique_lock lock(mutex);

    if (std::ranges::find(next_tracks, next) == next_tracks.end())
    {
        next_tracks.push_back(next);
    }
}

void Track::add_prev_track(Track *prev)
{
    if (prev == nullptr)
    {
        return;
    }

    std::unique_lock lock(mutex);

    if (std::ranges::find(prev_tracks, prev) == prev_tracks.end())
    {
        prev_tracks.push_back(prev);
    }
}

void Track::remove_next_track(Track *next)
{
    std::unique_lock lock(mutex);
    std::erase(next_tracks, next);
}

void Track::remove_prev_track(Track *prev)
{
    std::unique_lock lock(mutex);
    std::erase(prev_tracks, prev);
}

void Track::add_outbound_switch(Switch *sw)
{
    if (sw == nullptr)
    {
        return;
    }

    std::unique_lock lock(mutex);

    if (std::ranges::find(outbound_switches, sw) == outbound_switches.end())
    {
        outbound_switches.push_back(sw);
    }
}

void Track::add_inbound_switch(Switch *sw)
{
    if (sw == nullptr)
    {
        return;
    }

    std::unique_lock lock(mutex);

    if (std::ranges::find(inbound_switches, sw) == inbound_switches.end())
    {
        inbound_switches.push_back(sw);
    }
}

void Track::remove_outbound_switch(Switch *sw)
{
    std::unique_lock lock(mutex);
    std::erase(outbound_switches, sw);
}

void Track::remove_inbound_switch(Switch *sw)
{
    std::unique_lock lock(mutex);
    std::erase(inbound_switches, sw);
}