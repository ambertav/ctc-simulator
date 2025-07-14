#pragma once

class Train;
class Platform;
class Signal;

class Track
{
private:
    const int id;
    int duration;
    bool occupied;
    Signal *const signal;
    Train *current_train;
    Track *next;
    Track *prev;

public:
    Track(int i, Signal *s, int d = 1);

    int get_id() const;
    int get_duration() const;
    Signal *get_signal() const;
    const Train *get_occupying_train() const;
    Track *get_next() const;
    Track *get_prev() const;

    bool is_occupied() const;
    virtual bool is_platform() const;

    void set_next(Track *next);
    void set_prev(Track *prev);

    virtual bool allow_entry() const;
    void release_train();
    void accept_entry(Train *train);
};