#include <ranges>
#include <algorithm>

#include "core/switch.h"

Switch::Switch(int i) : id(i), in_progress(false) {}

int Switch::get_id() const
{
    return id;
}

const std::vector<Track *> &Switch::get_approach_tracks() const
{
    std::shared_lock lock(mutex);
    return approach_tracks;
}

const std::vector<Track *> &Switch::get_departure_tracks() const
{
    std::shared_lock lock(mutex);
    return departure_tracks;
}

const std::unordered_map<Track *, Track *> &Switch::get_links() const
{
    std::shared_lock lock(mutex);
    return links;
}

Track *Switch::get_link(Track *input) const
{
    std::shared_lock lock(mutex);

    auto it{links.find(input)};
    if (it != links.end())
    {
        return it->second;
    }
    else
    {
        return nullptr;
    }
}

bool Switch::ready() const
{
    return !in_progress.load(std::memory_order_acquire);
}

bool Switch::set_link(Track *input, Track *output)
{
    std::unique_lock lock(mutex);

    if (!ready())
    {
        return false;
    }

    if (std::ranges::find(approach_tracks, input) == approach_tracks.end() || std::ranges::find(departure_tracks, output) == departure_tracks.end())
    {
        return false;
    }

    begin_switching();
    links[input] = output;
    complete_switching();

    return true;
}

void Switch::add_approach_track(Track *tr)
{
    if (tr == nullptr)
    {
        return;
    }

    std::unique_lock lock(mutex);

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

    std::unique_lock lock(mutex);
    if (std::ranges::find(departure_tracks, tr) == departure_tracks.end())
    {
        departure_tracks.push_back(tr);
    }
}

void Switch::remove_approach_track(Track *tr)
{
    std::unique_lock lock(mutex);
    std::erase(approach_tracks, tr);
}

void Switch::remove_departure_track(Track *tr)
{
    std::unique_lock lock(mutex);
    std::erase(departure_tracks, tr);
}

void Switch::begin_switching()
{
    in_progress.store(true, std::memory_order_release);
}

void Switch::complete_switching()
{
    in_progress.store(false, std::memory_order_release);
}
