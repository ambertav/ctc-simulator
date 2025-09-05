#include "core/signal.h"

Signal::Signal(int i, int t) : id(i), state(SignalState::RED), target_id(t) {}

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

SignalState Signal::get_state() const
{
    return state;
}

bool Signal::change_state(SignalState new_state)
{
    bool changed {state != new_state};
    state = new_state;
    return changed;
}
