#include "core/signal.h"
#include "core/platform.h"

Platform::Platform(int i, int dw, Signal *si, const Station *st, Direction dir)
    : Track(i, si, dw), station(st), direction(dir) {}

const Station *Platform::get_station() const
{
    return station;
}

Direction Platform::get_direction() const
{
    return direction;
}

bool Platform::is_platform() const
{
    return true;
}

void Platform::set_direction(Direction dir)
{
    direction = dir;
}