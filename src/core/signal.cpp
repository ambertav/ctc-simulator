#include "core/signal.h"

Signal::Signal(int i) : id(i), track(nullptr), state(SignalState::RED), functional(true), failure_timer(0) {}

int Signal::get_id() const
{
    return id;
}

Track *Signal::get_track() const
{
    return track;
}

void Signal::set_track(Track *tr)
{
    track = tr;
}

void Signal::set_failure(int failure)
{
    if (failure > 0)
    {
        failure_timer = failure;
        functional = false;
    }
}

void Signal::update_repair()
{
    if (failure_timer > 0)
    {
        --failure_timer;

        if (failure_timer == 0)
        {
            functional = true;
        }
    }
}

bool Signal::is_red() const
{
    return state == SignalState::RED;
}

bool Signal::is_yellow() const
{
    return state == SignalState::YELLOW;
}

bool Signal::is_green() const
{
    return state == SignalState::GREEN;
}

bool Signal::is_functional() const
{
    return functional;
}

SignalState Signal::get_state() const
{
    return state;
}

bool Signal::change_state(SignalState new_state)
{
    bool changed{state != new_state};
    state = new_state;
    return changed;
}
