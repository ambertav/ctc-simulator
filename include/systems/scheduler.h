#pragma once

#include <string>
#include <fstream>

#include "map/graph.h"
#include "enums/direction.h"

class Scheduler
{
private:
    std::ofstream outfile;
    int spawn_tick_gap;
    int dwell_time;
    int number_of_trains;

public:
    Scheduler(const std::string &file_path, int stg = 2, int dt = 2, int nt = 3);
    void create_schedule(const Transit::Map::Path &path);
};