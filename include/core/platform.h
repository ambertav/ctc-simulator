/**
 * for details on design, see:
 * docs/platform.md
 */

#pragma once

#include "core/track.h"
#include "enums/transit_types.h"

class Station;
class Train;
class Signal;

class Platform : public Track
{
private:
    const Station *const station;
    Direction direction;

public:
    Platform(int i, Signal *si, const Station *st, Direction dir, int dw = 2, std::unordered_set<TrainLine> lines = {});

    const Station *get_station() const;
    virtual const Direction &get_direction() const;

    bool is_platform() const override;
};