/**
 * for details on design, see:
 * docs/station.md
 */

#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <span>
#include <optional>
#include <mutex>

#include "enum/transit_types.h"

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

    mutable std::shared_mutex mutex;

public:
    Station(int i, const std::string &n, bool y, const std::vector<TrainLine> &l);

    int get_id() const;
    const std::string &get_name() const;
    const std::vector<TrainLine> &get_train_lines() const;
    const std::vector<Platform *>& get_platforms() const;

    bool is_yard() const;
    
    void add_platform(Platform* platform);
    std::optional<Platform *> select_platform(Direction dir, TrainLine line) const;
};