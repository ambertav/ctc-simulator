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
    G,
    L,
    J,
    Z,
    S,
    SIR
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
    case TrainLine::G:
        return os << "G";
    case TrainLine::L:
        return os << "L";
    case TrainLine::J:
        return os << "J";
    case TrainLine::Z:
        return os << "Z";
    case TrainLine::S:
        return os << "Shuttle";
    case TrainLine::SIR:
        return os << "Station Island Railway";
    default:
        return os << "Unknown";
    }
}

inline TrainLine subway_train_line_from_string(const std::string &str)
{
    static const std::unordered_map<std::string, TrainLine> subway_mapping{
        {"1", TrainLine::ONE},
        {"2", TrainLine::TWO},
        {"3", TrainLine::THREE},
        {"4", TrainLine::FOUR},
        {"5", TrainLine::FIVE},
        {"6", TrainLine::SIX},
        {"7", TrainLine::SEVEN},
        {"A", TrainLine::A},
        {"C", TrainLine::C},
        {"E", TrainLine::E},
        {"N", TrainLine::N},
        {"Q", TrainLine::Q},
        {"R", TrainLine::R},
        {"W", TrainLine::W},
        {"B", TrainLine::B},
        {"D", TrainLine::D},
        {"F", TrainLine::F},
        {"M", TrainLine::M},
        {"G", TrainLine::G},
        {"L", TrainLine::L},
        {"J", TrainLine::J},
        {"Z", TrainLine::Z},
        {"S", TrainLine::S},
        {"SIR", TrainLine::SIR},
    };

    auto it = subway_mapping.find(str);
    if (it != subway_mapping.end())
    {
        return it->second;
    }
    else
    {
        throw std::invalid_argument("Invalid train line: " + str);
    }
}