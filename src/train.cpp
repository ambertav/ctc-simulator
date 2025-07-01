#include "core/track.h"
#include "core/platform.h"
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
        std::cout << "Invalid, train " << id << " cannot move to nullptr\n";
        return false;
    }
    else if (!next->allow_entry())
    {
        std::cout << "Train " << id << " cannot move to track " << next->get_id() << "\n";
        return false;
    }
    else
    {
        if (current_track)
            current_track->release_train();

        current_track = next;
        next->accept_entry(this);

        if (next->is_platform())
        {
            status = TrainStatus::ARRIVING;
            Platform *platform = static_cast<Platform *>(next);
            std::cout << "Train " << id << " is arriving at Station " << platform->get_station() << " on platform " << platform->get_id() << "\n";
        }
        else
        {
            status = TrainStatus::MOVING;
            std::cout << "Train " << id << " is moving to Track " << next->get_id() << "\n";
        }

        return true;
    }
}

void Train::update_status(TrainStatus status)
{
    this->status = status;
    std::cout << "Train " << id << " is now " << status << "\n";
}

void Train::update_destination(Platform *destination)
{
    this->destination = destination;
    std::cout << "Train " << id << " is heading to " << destination->get_station() << ", on platforn" << destination->get_id() << "\n";
}
