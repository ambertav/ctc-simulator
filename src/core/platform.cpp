#include "core/signal.h"
#include "core/platform.h"

Platform::Platform(int i, Signal *si, const Station *st, Direction d)
    : Track(i, si), station(st), direction(d) {}

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