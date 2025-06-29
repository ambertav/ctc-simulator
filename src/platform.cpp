#include "core/signal.h"
#include "core/platform.h"

Platform::Platform(int i, const Signal *si, const Station *st, Direction d)
    : id(i), occupied(false), signal(si), station(st), direction(d), current_train(nullptr), next_track(nullptr), prev_track(nullptr) {}

int Platform::get_id() const
{
    return id;
}

const Signal *Platform::get_signal() const
{
    return signal;
}

const Station *Platform::get_station() const
{
    return station;
}

const Train *Platform::get_occupying_train() const
{
    return current_train;
}

Direction Platform::get_direction() const
{
    return direction;
}

Track *Platform::get_next_track() const
{
    return next_track;
}

Track *Platform::get_prev_track() const
{
    return prev_track;
}

bool Platform::is_occupied() const
{
    return occupied;
}

void Platform::set_next_track(Track *next)
{
    next_track = next;
}

void Platform::set_prev_track(Track *prev)
{
    prev_track = prev;
}

void Platform::set_direction(Direction dir)
{
    direction = dir;
}

bool Platform::allow_entry() const
{
    if (occupied)
        return false;
    if (signal && !signal->is_green())
        return false;

    return true;
}

void Platform::release_train()
{
    occupied = false;
    current_train = nullptr;
}

void Platform::accept_entry(Train *train)
{
    if (train == nullptr)
    {
        std::cout << "Invalid, platform " << id << " cannot accept entry of nullptr\n";
        return;
    }
    occupied = true;
    current_train = train;
}