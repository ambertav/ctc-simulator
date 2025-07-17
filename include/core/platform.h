#pragma once

#include "core/track.h"
#include "enums/transit_types.hpp"

class Station;
class Train;
class Signal;

class Platform : public Track
{
private:
    const Station *const station;
    Direction direction;

public:
    Platform(int i, int dw, Signal *si, const Station *st, Direction dir);

    const Station *get_station() const;
    virtual Direction get_direction() const;

    bool is_platform() const override;

    void set_direction(Direction dir);
};