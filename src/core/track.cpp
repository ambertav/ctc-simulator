/**
 * for details on design, see:
 * docs/track.md
 */

#include <ranges>
#include <algorithm>

#include "core/signal.h"
#include "core/track.h"

Track::Track(int i, Signal *s, int d, std::unordered_set<TrainLine> lines)
    : id(i), duration(std::max(1, d)), occupied(false), signal(s), current_train(nullptr), train_lines(std::move(lines)), outbound_switch(nullptr), inbound_switch(nullptr) {}

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

Switch *Track::get_outbound_switch() const
{
    return outbound_switch.load(std::memory_order_acquire);
}

Switch *Track::get_inbound_switch() const
{
    return inbound_switch.load(std::memory_order_acquire);
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

Track *Track::get_next_track(TrainLine line) const
{
    std::shared_lock lock(mutex);

    for (auto *next : next_tracks)
    {
        if (next->supports_train_line(line))
        {
            return next;
        }
    }

    return nullptr;
}

Track *Track::get_prev_track(TrainLine line) const
{
    std::shared_lock lock(mutex);

    for (auto *prev : prev_tracks)
    {
        if (prev->supports_train_line(line))
        {
            return prev;
        }
    }

    return nullptr;
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

    outbound_switch.store(sw, std::memory_order_release);
}

void Track::add_inbound_switch(Switch *sw)
{
    if (sw == nullptr)
    {
        return;
    }

    inbound_switch.store(sw, std::memory_order_release);
}

void Track::remove_outbound_switch()
{
    outbound_switch.store(nullptr, std::memory_order_release);
}

void Track::remove_inbound_switch()
{
    inbound_switch.store(nullptr, std::memory_order_release);
}
