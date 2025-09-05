#pragma once

#include "enum/signal_state.h"

class Track;
class Platform;

class Signal
{
private:
    const int id;
    SignalState state;
    const int target_id;

public:
    Signal(int i, int t);

    int get_id() const;
    int get_target() const;

    bool is_red() const;
    bool is_yellow() const;
    virtual bool is_green() const;

    SignalState get_state() const;
    bool change_state(SignalState new_state);
};