#include "core/signal.h"
#include "core/track.h"

Track::Track(int i, Signal *s, int d) : id(i), distance(d), occupied(false), signal(s), current_train(nullptr), next(nullptr), prev(nullptr) {}

int Track::get_id() const
{
    return id;
}

Signal *Track::get_signal() const
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

bool Track::is_occupied() const
{
    return occupied;
}

bool Track::is_platform() const
{
    return false;
}

void Track::set_next(Track *next)
{
    this->next = next;
}

void Track::set_prev(Track *prev)
{
    this->prev = prev;
}

bool Track::allow_entry() const
{
    if (occupied)
        return false;
    if (signal && !signal->is_green())
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