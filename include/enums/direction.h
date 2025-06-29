#pragma once

#include <iostream>

enum class Direction
{
    UPTOWN,
    DOWNTOWN
};

inline std::ostream &operator<<(std::ostream &os, Direction direction)
{
    switch (direction)
    {
    case Direction::UPTOWN:
        return os << "uptown";
    case Direction::DOWNTOWN:
        return os << "downtown";
    }
}