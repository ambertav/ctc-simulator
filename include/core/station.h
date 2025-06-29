#pragma once

#include <string>
#include <vector>
#include <span>

#include "enums/train_line.h"
#include "enums/direction.h"

class Platform;
class Train;

class Station
{
private:
    const int id;
    const std::string name;
    std::vector<TrainLine> train_lines;
    std::vector<std::unique_ptr<Platform>> platforms;

public:
    Station(int i, const std::string &n, const std::vector<TrainLine> &l);
    Station(Station &&) noexcept = default;

    int get_id() const;
    const std::string &get_name() const;
    const std::vector<TrainLine> &get_lines() const;
    std::vector<Platform *> get_platforms() const;

    void add_platform(std::unique_ptr<Platform> platform);

    std::optional<Platform *> find_available_platform(Direction dir) const;
    std::vector<Platform *> get_platforms_by_direction(Direction dir) const;
};