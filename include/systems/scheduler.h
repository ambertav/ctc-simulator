#pragma once

#include <string>
#include <fstream>

#include "map/base.h"
#include "enums/direction.h"

class Scheduler
{
private:
    int spawn_tick_gap;
    int dwell_time;
    int number_of_trains;

public:
    Scheduler(int stg = 2, int dt = 1, int nt = 3) : spawn_tick_gap(stg), dwell_time(dt), number_of_trains(nt) {}
    void create_schedule(const Transit::Map::Path &path, const std::string &outfile) const;
};