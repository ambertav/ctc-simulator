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
    if (signal->is_yellow())
    {
        return static_cast<int>(std::ceil(duration * 1.3));
    }
    else
    {
        return duration;
    }
}

Signal *Track::get_signal() const
{
    return signal;
}

const Train *Track::get_occupying_train() const
{
    return current_train;
}

const std::vector<Track *> &Track::get_next_tracks() const
{
    return next_tracks;
}

const std::vector<Track *> &Track::get_prev_tracks() const
{
    return prev_tracks;
}

Switch *Track::get_outbound_switch() const
{
    return outbound_switch;
}

Switch *Track::get_inbound_switch() const
{
    return inbound_switch;
}

bool Track::is_occupied() const
{
    return occupied;
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
    Track* fallback{nullptr};

    for (Track* tr : next_tracks)
    {
        if (tr->supports_train_line(line))
        {
            if (!tr->is_occupied())
            {
                return tr;
            }
            else
            {
                fallback = tr;
            }
        }
    }

    return fallback;
}

Track *Track::get_prev_track(TrainLine line) const
{
    Track* fallback{nullptr};

    for (Track* tr : prev_tracks)
    {
        if (tr->supports_train_line(line))
        {
            if (!tr->is_occupied())
            {
                return tr;
            }
            else
            {
                fallback = tr;
            }
        }
    }

    return fallback;
}


bool Track::accept_entry(Train *train)
{
    if (train == nullptr)
    {
        return false;
    }

    if (occupied || signal->is_red())
    {
        return false;
    }

    occupied = true;
    current_train = train;
    return true;
}

void Track::release_train()
{
    current_train = nullptr;
    occupied = false;
}

void Track::add_train_line(TrainLine line)
{
    train_lines.insert(line);
}

void Track::add_next_track(Track *next)
{
    if (next == nullptr)
    {
        return;
    }

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

    if (std::ranges::find(prev_tracks, prev) == prev_tracks.end())
    {
        prev_tracks.push_back(prev);
    }
}

void Track::add_outbound_switch(Switch *sw)
{
    if (sw == nullptr)
    {
        return;
    }

    outbound_switch = sw;
}

void Track::add_inbound_switch(Switch *sw)
{
    if (sw == nullptr)
    {
        return;
    }

    inbound_switch = sw;
}