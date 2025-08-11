#pragma once

#include <iostream>

enum class SignalState
{
    RED,
    YELLOW,
    GREEN
};

inline std::ostream &operator<<(std::ostream &os, SignalState state)
{
    switch (state)
    {
    case SignalState::RED:
        return os << "red";
    case SignalState::YELLOW:
        return os << "yellow";
    case SignalState::GREEN:
        return os << "green";
    }
}