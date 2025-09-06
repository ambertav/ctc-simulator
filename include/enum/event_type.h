#pragma once

#include <iostream>

enum class EventType
{
    ARRIVAL,
    DEPARTURE,
};

inline std::string event_type_to_string(EventType type)
{
    switch (type)
    {
    case EventType::ARRIVAL:
        return "arrival";
    case EventType::DEPARTURE:
        return "departure";
    default:
        return "unknown";
    }
}

inline std::ostream &operator<<(std::ostream &os, const EventType &type)
{
    return os << event_type_to_string(type);
}