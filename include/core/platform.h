#pragma once

#include "core/track.h"
#include "enums/direction.h"

class Station;
class Train;
class Signal;

class Platform : public Track
{
private:
    const Station *const station;
    Direction direction;

public:
    Platform(int i, Signal *si, const Station *st, Direction d);

    const Station *get_station() const;
    virtual Direction get_direction() const;

    bool is_platform() const override;

    void set_direction(Direction dir);
};