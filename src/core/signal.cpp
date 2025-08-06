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
    return state.load(std::memory_order_acquire) == SignalState::RED;
}

bool Signal::is_yellow() const
{
    return state.load(std::memory_order_acquire) == SignalState::YELLOW;
}

bool Signal::is_green() const
{
    return state.load(std::memory_order_acquire) == SignalState::GREEN;
}

SignalState Signal::get_state() const
{
    return state.load(std::memory_order_acquire);
}

bool Signal::change_state(SignalState new_state)
{
    SignalState old_state {state.exchange(new_state, std::memory_order_acq_rel)};
    return old_state != new_state;
}
