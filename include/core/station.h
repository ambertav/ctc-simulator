#pragma once

#include <string>
#include <vector>
#include <span>
#include <optional>

#include "enums/transit_types.hpp"

class Platform;
class Train;

class Station
{
private:
    const int id;
    const std::string name;
    const bool yard;
    std::vector<TrainLine> train_lines;
    std::vector<Platform*> platforms;

public:
    Station(int i, const std::string &n, bool y, const std::vector<TrainLine> &l);

    int get_id() const;
    const std::string &get_name() const;
    const std::vector<TrainLine> &get_lines() const;
    std::vector<Platform *> get_platforms() const;

    bool is_yard() const;
    
    void add_platform(Platform* platform);

    std::optional<Platform *> find_available_platform(Direction dir) const;
    std::vector<Platform *> get_platforms_by_direction(Direction dir) const;
};