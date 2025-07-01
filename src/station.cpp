#include "core/platform.h"
#include "core/station.h"

#include <ranges>

Station::Station(int i, const std::string &n, bool y, const std::vector<TrainLine> &l)
    : id(i), name(n), yard(y), train_lines(l) {}

int Station::get_id() const
{
    return id;
}

const std::string &Station::get_name() const
{
    return name;
}

const std::vector<TrainLine> &Station::get_lines() const
{
    return train_lines;
}

std::vector<Platform *> Station::get_platforms() const
{
    std::vector<Platform *> output;
    output.reserve(platforms.size());
    for (const auto &p : platforms)
    {
        output.push_back(p.get());
    }

    return output;
}

bool Station::is_yard() const
{
    return yard;
}

void Station::add_platform(std::unique_ptr<Platform> platform)
{
    platforms.push_back(std::move(platform));
}

std::optional<Platform *> Station::find_available_platform(Direction dir) const
{
    for (const auto &p : platforms)
    {
        if (p->get_direction() == dir && p->allow_entry())
            return p.get();
    }

    return std::nullopt;
}

std::vector<Platform *> Station::get_platforms_by_direction(Direction dir) const
{
    std::vector<Platform *> output;
    output.reserve(platforms.size());

    for (const auto &p : platforms)
    {
        if (p->get_direction() == dir)
            output.push_back(p.get());
    }

    return output;
}