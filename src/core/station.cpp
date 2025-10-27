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
    return train_lines;
}

const std::vector<Platform *> &Station::get_platforms() const
{
    return platforms;
}

bool Station::is_yard() const
{
    return yard;
}

void Station::add_platform(Platform *platform)
{
    platforms.push_back(platform);
}

std::vector<Platform *> Station::select_platforms(Direction dir, TrainLine line) const
{
    std::vector<Platform *> selected{};

    std::ranges::copy_if(platforms, std::back_inserter(selected), [&](Platform *p)
                         { return directions_equal(p->get_direction(), dir) && p->supports_train_line(line); });

    return selected;
}