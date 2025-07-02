#include "core/signal.h"

Signal::Signal(int i, int t) : id(i), state(SignalState::RED), target_id(t), delay(0) {}

int Signal::get_id() const
{
    return id;
}

int Signal::get_target() const
{
    return target_id;
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

int Signal::get_delay() const
{
    return delay;
}

void Signal::set_delay(int d)
{
    if (d < 0)
        return;
    else
        delay = d;
}

bool Signal::is_delayed() const
{
    return delay > 0;
}

void Signal::decrement_delay()
{
    if (delay == 0)
        return;
    else
        --delay;
}

bool Signal::change_state(SignalState state)
{
    bool changed = this->state != state;
    this->state = state;
    return changed;
}
