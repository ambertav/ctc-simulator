#include "core/train.h"

Train::Train(int i, TrainLine l, ServiceType t, Direction d)
    : id(i), dwell_timer(0), punctuality_delta(0), train_line(l), service_type(t), status(TrainStatus::IDLE), direction(d), current_track(nullptr) {}

int Train::get_id() const
{
    return id;
}

int Train::get_dwell() const
{
    return dwell_timer;
}

int Train::get_lateness() const
{
    return punctuality_delta;
}

std::string_view Train::get_headsign() const
{
    return headsign;
}

TrainLine Train::get_train_line() const
{
    return train_line;
}

ServiceType Train::get_service_type() const
{
    return service_type;
}

TrainStatus Train::get_status() const
{
    return status;
}

Direction Train::get_direction() const
{
    return direction;
}

Track *Train::get_current_track() const
{
    return current_track;
}

void Train::set_lateness(int delta)
{
    punctuality_delta = delta;
}

void Train::set_headsign(std::string trip_headsign)
{
    headsign = trip_headsign;
}

void Train::add_dwell(int additional_dwell)
{
    if (additional_dwell > 0)
    {
        dwell_timer += additional_dwell;
    }
}

bool Train::is_idle() const
{
    return status == TrainStatus::IDLE;
}

bool Train::is_active() const
{
    return status != TrainStatus::IDLE && status != TrainStatus::OUTOFSERVICE;
}

bool Train::is_arriving() const
{
    return status == TrainStatus::ARRIVING;
}

bool Train::is_departing() const
{
    return status == TrainStatus::DEPARTING;
}

bool Train::is_late() const
{
    return punctuality_delta > 0;
}

bool Train::request_movement()
{
    if (dwell_timer <= 0)
    {
        return true;
    }
    else
    {
        --dwell_timer;
        return false;
    }
}

bool Train::move_to_track(Track *to)
{
    if (to == nullptr)
    {
        return false;
    }
    
    Track *from{current_track};

    bool moved {to->accept_entry(this)};
    if (!moved)
    {
        return false;
    }

    if (from)
    {
        from->release_train();
    }

    current_track = to;

    if (to->is_platform())
    {
        status = TrainStatus::ARRIVING;
    }
    else if (from && from->is_platform())
    {
        status = TrainStatus::DEPARTING;
    }
    else
    {
        status = TrainStatus::MOVING;
    }

    dwell_timer = to->get_duration();
    return true;
}

void Train::spawn(Platform *yard_platform)
{
    bool spawned{yard_platform->accept_entry(this)};
    if (spawned)
    {
        current_track = yard_platform;
        status = TrainStatus::READY;
    }
}

void Train::despawn()
{
    current_track->release_train();
    current_track = nullptr;
    status = TrainStatus::OUTOFSERVICE;
}