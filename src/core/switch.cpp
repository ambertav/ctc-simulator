/**
 * for details on design, see:
 * docs/switch.md
 */

#include <ranges>
#include <algorithm>

#include "core/switch.h"

Switch::Switch(int i) : id(i), failure_timer(0), functional(true) {}

int Switch::get_id() const
{
    return id;
}

const std::vector<Track *> &Switch::get_approach_tracks() const
{
    return approach_tracks;
}

const std::vector<Track *> &Switch::get_departure_tracks() const
{
    return departure_tracks;
}

Track *Switch::get_link(Track *input) const
{
    if (link.first == input)
    {
        return link.second;
    }
    else
    {
        return nullptr;
    }
}

bool Switch::set_link(Track *input, Track *output)
{
    if (std::ranges::find(approach_tracks, input) == approach_tracks.end() || std::ranges::find(departure_tracks, output) == departure_tracks.end())
    {
        return false;
    }
    link = std::make_pair(input, output);
    return true;
}

void Switch::set_failure(int failure)
{
    if (failure > 0)
    {
        failure_timer = failure;
        functional = false;
    }
}

void Switch::update_repair()
{
    if (failure_timer > 0)
    {
        --failure_timer;
    }

    if (failure_timer == 0)
    {
        functional = true;
    }
}

bool Switch::is_functional() const
{
    return functional;
}

void Switch::add_approach_track(Track *tr)
{
    if (tr == nullptr)
    {
        return;
    }

    if (std::ranges::find(approach_tracks, tr) == approach_tracks.end())
    {
        approach_tracks.push_back(tr);
    }
}

void Switch::add_departure_track(Track *tr)
{
    if (tr == nullptr)
    {
        return;
    }

    if (std::ranges::find(departure_tracks, tr) == departure_tracks.end())
    {
        departure_tracks.push_back(tr);
    }
}