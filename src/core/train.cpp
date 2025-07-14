#include "core/train.h"

#include <iostream>

Train::Train(int i, TrainLine l, ServiceType t, Track *ct) : id(i), delay(0), line(l), type(t), current_track(ct), status(TrainStatus::IDLE), destination(nullptr) {}

int Train::get_id() const
{
    return id;
}

int Train::get_delay() const
{
    return delay;
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

bool Train::is_active() const 
{
    return status != TrainStatus::IDLE && status != TrainStatus::OUTOFSERVICE;
}

void Train::add_delay(int additional_delay)
{
    if (additional_delay > 0)
    {
        delay += additional_delay;
    }
}

bool Train::request_movement()
{
    if (delay <= 0)
    {
        return true;
    }
    else
    {
        --delay;
        return false;
    }
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

        delay = current_track->get_duration();

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
