#pragma once

#include "enums/train_line.h"
#include "enums/service_type.h"
#include "enums/train_status.h"

#include "core/track.h"
#include "core/platform.h"
#include "core/station.h"

class Track;
class Platform;

class Train
{
private:
    const int id;
    const TrainLine line;
    ServiceType type;
    Track *current_track;
    TrainStatus status;
    Platform *destination;

public:
    Train(int i, TrainLine l, ServiceType t, Track* ct);

    int get_id() const;
    TrainLine get_line() const;
    ServiceType get_type() const;
    Track *get_current_track() const;
    TrainStatus get_status() const;
    Platform *get_destination() const;

    bool can_advance() const;
    bool move_to_track();
    void spawn(Platform* yard);
    void despawn();
    
    void update_status(TrainStatus status);
    void update_destination(Platform* destination);
};