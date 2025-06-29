#pragma once

class Train;
class Platform;
class Signal;

class Track
{
private:
    const int id;
    bool occupied;
    bool switch_on;
    const Signal *const signal;
    Train *current_train;
    Track *next;                // main track
    Track *switch_next;         // siding track
    Track *prev;

public:
    Track(int i, const Signal *s);

    int get_id() const;
    const Signal* get_signal() const;
    const Train* get_occupying_train() const;
    Track *get_next() const;
    Track *get_prev() const;
    Track *get_switch_next() const;
    Track *get_active_next() const;

    bool is_occupied() const;
    bool is_switch_on() const;

    void set_next(Track *next);
    void set_prev(Track *prev);
    void set_switch_next(Track *next);

    void toggle_switch();
    virtual bool allow_entry() const;
    void release_train();
    void accept_entry(Train *train);
};