/**
 * for details on design, see:
 * docs/station.md
 */

#include "core/platform.h"
#include "core/station.h"

#include <ranges>
#include <algorithm>

Station::Station(int i, const std::string &n, bool y, const std::unordered_set<TrainLine> &l)
    : id(i), name(n), yard(y), train_lines(l) {}

int Station::get_id() const
{
    return id;
}

const std::string &Station::get_name() const
{
    return name;
}

const std::unordered_set<TrainLine> &Station::get_train_lines() const
{
    std::shared_lock lock(mutex);
    return train_lines;
}

const std::vector<Platform *> &Station::get_platforms() const
{
    std::shared_lock lock(mutex);
    return platforms;
}

bool Station::is_yard() const
{
    return yard;
}

void Station::add_platform(Platform *platform)
{
    std::unique_lock lock(mutex);
    platforms.push_back(platform);
}

std::optional<Platform *> Station::select_platform(Direction dir, TrainLine line) const
{
    std::shared_lock lock(mutex);

    auto it{std::ranges::find_if(platforms, [&](Platform *p)
                                 { return directions_equal(p->get_direction(), dir) && p->supports_train_line(line); })};

    if (it != platforms.end())
    {
        return *it;
    }
    else
    {
        return std::nullopt;
    }
}