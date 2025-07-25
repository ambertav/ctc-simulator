#pragma once

#include <string>
#include <fstream>

#include "map/graph.h"
#include "enums/transit_types.h"

class Scheduler
{
private:
    std::ofstream outfile;
    int spawn_tick_gap;
    int dwell_time;
    int number_of_trains;

public:
    Scheduler(const std::string &file_path, int stg = 4, int dt = 2, int nt = 6);
    void create_schedule(const Transit::Map::Path &path);
};