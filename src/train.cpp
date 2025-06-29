#include "core/track.h"
#include "core/platform.h"
#include "core/train.h"

#include <iostream>

Train::Train(int i, TrainLine l, ServiceType t) : id(i), line(l), type(t), current_track(nullptr), status(TrainStatus::IDLE), destination(nullptr) {}

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

bool Train::move_to_track(Track *track)
{
    if (track == nullptr) {
        std::cout << "Invalid, train " << id << " cannot move to nullptr\n";
        return false;
    }
    else if (!track->allow_entry())
    {
        std::cout << "Train " << id << " cannot move to track " << track->get_id() << "\n";
        return false;
    }
    else
    {
        if (current_track)
            current_track->release_train();

        current_track = track;
        track->accept_entry(this);
        status = TrainStatus::MOVING;

        std::cout << "Train " << id << " is moving to Track " << track->get_id() << "\n";
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
