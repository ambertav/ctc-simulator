#pragma once

#include <iostream>

enum class TrainStatus
{
    ARRIVING,
    DEPARTING,
    IDLE,
    MOVING,
    UNKNOWN
};

inline std::ostream &operator<<(std::ostream &os, TrainStatus status)
{
    switch (status)
    {
    case TrainStatus::ARRIVING:
        return os << "arriving";
    case TrainStatus::DEPARTING:
        return os << "departing";
    case TrainStatus::IDLE:
        return os << "idle";
    case TrainStatus::MOVING:
        return os << "moving";
    case TrainStatus::UNKNOWN:
        return os << "unknown";
    }
}