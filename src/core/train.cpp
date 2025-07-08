#include "core/train.h"

#include <iostream>

Train::Train(int i, TrainLine l, ServiceType t, Track *ct) : id(i), line(l), type(t), current_track(ct), status(TrainStatus::IDLE), destination(nullptr) {}

int Train::get_id() const
{
    return id;
}

TrainLine Train::get_line() const
{
    return line;
}

ServiceType Train::get_type() const
{
    return type;
}

Track *Train::get_current_track() const
{
    return current_track;
}

TrainStatus Train::get_status() const
{
    return status;
}

Platform *Train::get_destination() const
{
    return destination;
}

bool Train::can_advance() const
{
    Track *next = current_track ? current_track->get_next() : nullptr;
    if (!next)
        return false;
    return next->allow_entry();
}

bool Train::move_to_track()
{
    Track *next = current_track ? current_track->get_next() : nullptr;
    if (!next)
    {
        std::cerr << "Invalid, train " << id << " cannot move to nullptr\n";
        return false;
    }
    else if (!next->allow_entry())
    {
        std::cerr << "Train " << id << " cannot move to track " << next->get_id() << "\n";
        return false;
    }
    else
    {
        if (current_track)
        {
            current_track->release_train();
        }

        Track *from = current_track;
        current_track = next;
        current_track->accept_entry(this);

        if (current_track->is_platform())
        {
            status = TrainStatus::ARRIVING;
            Platform *platform = static_cast<Platform *>(current_track);
        }
        else if (from && from->is_platform())
        {
            status = TrainStatus::DEPARTING;
            Platform *platform = static_cast<Platform *>(from);
        }
        else
        {
            status = TrainStatus::MOVING;
        }

        return true;
    }
}

void Train::spawn(Platform *yard)
{
    current_track = yard;
    yard->accept_entry(this);
    status = TrainStatus::READY;
}

void Train::despawn()
{
    current_track->release_train();
    current_track = nullptr;
    status = TrainStatus::OUTOFSERVICE;
}

void Train::update_status(TrainStatus status)
{
    this->status = status;
}

void Train::update_destination(Platform *destination)
{
    this->destination = destination;
}
