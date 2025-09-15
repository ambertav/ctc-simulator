#include "core/signal.h"
#include "core/platform.h"

Platform::Platform(int i, Signal *si, const Station *st, Direction dir, int dw, std::unordered_set<TrainLine> lines)
    : Track(i, si, dw, lines), station(st), direction(dir) {}

const Station *Platform::get_station() const
{
    return station;
}

const Direction& Platform::get_direction() const
{
    return direction;
}

bool Platform::is_platform() const
{
    return true;
}