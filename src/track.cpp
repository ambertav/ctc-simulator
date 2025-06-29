#include "core/signal.h"
#include "core/track.h"

Track::Track(int i, const Signal *s) : id(i), occupied(false), switch_on(false), signal(s), current_train(nullptr), next(nullptr), switch_next(nullptr), prev(nullptr) {}

int Track::get_id() const
{
    return id;
}

const Signal *Track::get_signal() const
{
    return signal;
}

const Train *Track::get_occupying_train() const
{
    return current_train;
}

Track *Track::get_next() const
{
    return next;
}

Track *Track::get_prev() const
{
    return prev;
}

Track *Track::get_switch_next() const
{
    return switch_next;
}

Track *Track::get_active_next() const
{
    return switch_on ? switch_next : next;
}

bool Track::is_occupied() const
{
    return occupied;
}

bool Track::is_switch_on() const
{
    return switch_on;
}

void Track::set_next(Track *next)
{
    this->next = next;
}

void Track::set_prev(Track *prev)
{
    this->prev = prev;
}

void Track::set_switch_next(Track *next)
{
    switch_next = next;
}

void Track::toggle_switch()
{
    if (switch_next == nullptr || next == nullptr)
        return;
    switch_on = !switch_on;
}

bool Track::allow_entry() const
{
    if (occupied)
        return false;
    if (signal && !signal->is_green())
        return false;
    if (switch_on && switch_next == nullptr)
        return false;

    return true;
}

void Track::release_train()
{
    occupied = false;
    current_train = nullptr;
}

void Track::accept_entry(Train *train)
{
    if (train == nullptr)
    {
        std::cout << "Invalid, track " << id << " cannot accept entry of nullptr\n";
        return;
    }

    occupied = true;
    current_train = train;
}