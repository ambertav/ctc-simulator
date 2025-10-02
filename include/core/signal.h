#pragma once

#include "enum/signal_state.h"

class Track;

class Signal
{
private:
    const int id;
    SignalState state;
    Track* track;
    int failure_timer;
    bool functional;

public:
    Signal(int i);

    int get_id() const;
    Track* get_track() const;

    void set_track(Track* tr);

    void set_failure(int failure);
    void update_repair();
    
    bool is_red() const;
    bool is_yellow() const;
    virtual bool is_green() const;
    bool is_functional() const;
    
    SignalState get_state() const;
    bool change_state(SignalState new_state);
};