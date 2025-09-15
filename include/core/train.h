#pragma once

#include <string>
#include <string_view>

#include "enum/transit_types.h"
#include "enum/service_type.h"
#include "enum/train_status.h"

#include "core/track.h"
#include "core/platform.h"


class Train
{
private:
    const int id;
    int dwell_timer;
    int punctuality_delta;
    std::string headsign;
    const TrainLine train_line;
    ServiceType service_type;
    TrainStatus status;
    Direction direction;
    Track *current_track;

public:
    Train(int i, TrainLine l, ServiceType t, Direction d);

    int get_id() const;
    int get_dwell() const;
    int get_lateness() const;
    std::string_view get_headsign() const;
    TrainLine get_train_line() const;
    ServiceType get_service_type() const;
    TrainStatus get_status() const;
    Direction get_direction() const;
    Track *get_current_track() const;

    void set_lateness(int delta);
    void set_headsign(std::string trip_headsign);
    
    void add_dwell(int additional_dwell);

    bool is_idle() const;
    bool is_active() const;
    bool is_arriving() const;
    bool is_departing() const;
    bool is_late() const;

    bool request_movement();
    bool move_to_track(Track* to);

    void spawn(Platform *yard_platform);
    void despawn();
};