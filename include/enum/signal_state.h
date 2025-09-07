#pragma once

#include <iostream>

enum class SignalState
{
    RED,
    YELLOW,
    GREEN
};

inline std::string signal_state_to_string(SignalState state)
{
    switch (state)
    {
    case SignalState::RED:
        return "red";
    case SignalState::YELLOW:
        return "yellow";
    case SignalState::GREEN:
        return "green";
    }
}

inline std::ostream &operator<<(std::ostream &os, SignalState state)
{
    return os << signal_state_to_string(state);
}