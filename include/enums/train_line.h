#pragma once

#include <iostream>

enum class TrainLine
{
    ONE,
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX,
    SEVEN,
    A,
    C,
    E,
    N,
    Q,
    R,
    W,
    B,
    D,
    F,
    M,
    L,
    J,
    Z
};

inline std::ostream &operator<<(std::ostream &os, TrainLine line)
{
    switch (line)
    {
    case TrainLine::ONE:
        return os << "1";
    case TrainLine::TWO:
        return os << "2";
    case TrainLine::THREE:
        return os << "3";
    case TrainLine::FOUR:
        return os << "4";
    case TrainLine::FIVE:
        return os << "5";
    case TrainLine::SIX:
        return os << "6";
    case TrainLine::SEVEN:
        return os << "7";
    case TrainLine::A:
        return os << "A";
    case TrainLine::C:
        return os << "C";
    case TrainLine::E:
        return os << "E";
    case TrainLine::N:
        return os << "N";
    case TrainLine::Q:
        return os << "Q";
    case TrainLine::R:
        return os << "R";
    case TrainLine::W:
        return os << "W";
    case TrainLine::B:
        return os << "B";
    case TrainLine::D:
        return os << "D";
    case TrainLine::F:
        return os << "F";
    case TrainLine::M:
        return os << "M";
    case TrainLine::L:
        return os << "L";
    case TrainLine::J:
        return os << "J";
    case TrainLine::Z:
        return os << "Z";
    }
}