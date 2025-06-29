#pragma once

#include "enums/direction.h"

class Station;
class Train;
class Track;
class Signal;

class Platform
{
private:
    const int id;
    bool occupied;
    const Signal *const signal;
    const Station *const station;
    Direction direction;
    Train *current_train;
    Track *next_track;
    Track *prev_track;

public:
    Platform(int i, const Signal *si, const Station *st, Direction d);

    int get_id() const;
    const Signal* get_signal() const;
    const Station* get_station() const;
    const Train* get_occupying_train() const;
    virtual Direction get_direction() const;
    Track *get_next_track() const;
    Track *get_prev_track() const;

    bool is_occupied() const;

    void set_next_track(Track *next);
    void set_prev_track(Track *prev);
    void set_direction(Direction dir);

    virtual bool allow_entry() const;
    void release_train();
    void accept_entry(Train *train);
};