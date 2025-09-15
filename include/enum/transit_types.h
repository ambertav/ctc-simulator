#pragma once

#include <variant>
#include <optional>
#include <ranges>
#include <string>
#include <sstream>
#include <unordered_map>
#include <iostream>

namespace SUB
{
    enum class Direction
    {
        UPTOWN,
        DOWNTOWN,
        COUNT
    };
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
        GS,
        FS,
        H,
        SI,
        COUNT
    };
}

namespace MNR
{
    enum class Direction
    {
        INBOUND,
        OUTBOUND,
        COUNT
    };
    enum class TrainLine
    {
        HARLEM,
        HUDSON,
        NEW_HAVEN,
        COUNT
    };
}

namespace LIRR
{
    enum class Direction
    {
        WESTBOUND,
        EASTBOUND,
        COUNT
    };
    enum class TrainLine
    {
        BABYLON,
        FAR_ROCKAWAY,
        HEMPSTEAD,
        LONG_BEACH,
        MONTAUK,
        OYSTER_BAY,
        PORT_JEFFERSON,
        PORT_WASHINGTON,
        RONKONKOMA,
        WEST_HEMPSTEAD,
        COUNT
    };
}

namespace Generic
{
    enum class Direction
    {
        DEFAULT_A,
        DEFAULT_B,
        COUNT
    };

    enum class TrainLine
    {
        DEFAULT,
        COUNT
    };
}

using Direction = std::variant<SUB::Direction, MNR::Direction, LIRR::Direction, Generic::Direction>;
using TrainLine = std::variant<SUB::TrainLine, MNR::TrainLine, LIRR::TrainLine, Generic::TrainLine>;

///////////////////////
// DIRECTION HELPERS //
///////////////////////

inline bool directions_equal(const Direction &a, const Direction &b)
{
    if (a.index() != b.index())
    {
        return false;
    }

    return std::visit([&](const auto &lhs, const auto &rhs) -> bool
                      {
                        using T = std::decay_t<decltype(lhs)>;
                        using U = std::decay_t<decltype(rhs)>;
                        if constexpr (std::is_same_v<T, U>){
                            return lhs == rhs;
                        } else {
                            return false;
                        } }, a, b);
}

inline Direction infer_direction(const TrainLine &trainline, const std::pair<double, double> &from, const std::pair<double, double> &to)
{
    return std::visit([&](const auto &trainline) -> Direction
                      {
        using T = std::decay_t<decltype(trainline)>;

        auto check_latitude = [&](auto down, auto up)
        {
            return (from.first > to.first) ? down : up;
        };

        auto check_longitude = [&](auto west, auto east)
        {
            return (from.second > to.second) ? west : east;
        };

        if constexpr (std::is_same_v<T, SUB::TrainLine>)
        {
            return check_latitude(SUB::Direction::DOWNTOWN, SUB::Direction::UPTOWN);
        } 
        else if constexpr (std::is_same_v<T, MNR::TrainLine>)
        {
            return check_latitude(MNR::Direction::INBOUND, MNR::Direction::OUTBOUND);
        } 
        else if constexpr (std::is_same_v<T, LIRR::TrainLine>)
        {
            return check_longitude(LIRR::Direction::WESTBOUND, LIRR::Direction::EASTBOUND);
        }
        else 
        {
            return check_latitude(Generic::Direction::DEFAULT_A, Generic::Direction::DEFAULT_A);
        } }, trainline);
}

inline std::optional<Direction> direction_from_string(const std::string &str)
{
    static const std::unordered_map<std::string, Direction> direction_map{
        {"uptown", SUB::Direction::UPTOWN},
        {"downtown", SUB::Direction::DOWNTOWN},
        {"inbound", MNR::Direction::INBOUND},
        {"outbound", MNR::Direction::OUTBOUND},
        {"eastbound", LIRR::Direction::EASTBOUND},
        {"westbound", LIRR::Direction::WESTBOUND},
        {"defaulta", Generic::Direction::DEFAULT_A},
        {"defaultb", Generic::Direction::DEFAULT_B}};

    auto it = direction_map.find(str);
    if (it != direction_map.end())
    {
        return it->second;
    }
    else
    {
        return std::nullopt;
    }
}

inline std::ostream &operator<<(std::ostream &os, const Direction &direction)
{
    return std::visit([&os](const auto &dir) -> std::ostream &
                      {
                   using T = std::decay_t<decltype(dir)>;
                   if constexpr (std::is_same_v<T, SUB::Direction>)
                   {
                       switch (dir)
                       {
                        case SUB::Direction::UPTOWN: return os << "uptown";
                        case SUB::Direction::DOWNTOWN: return os << "downtown";
                        default: return os << "unknown subway direction";
                       }
                   }

                   else if constexpr (std::is_same_v<T, MNR::Direction>)
                   {
                       switch (dir)
                       {
                        case MNR::Direction::INBOUND: return os << "inbound";
                        case MNR::Direction::OUTBOUND: return os << "outbound";
                        default: return os << "unknown metro north direction";
                       }
                   }

                   else if constexpr (std::is_same_v<T, LIRR::Direction>)
                   {
                       switch (dir)
                       {
                        case LIRR::Direction::EASTBOUND: return os << "eastbound";
                        case LIRR::Direction::WESTBOUND: return os << "westbound";
                        default: return os << "unknown long island railroad direction";
                       }
                   }

                   else if constexpr (std::is_same_v<T, Generic::Direction>)
                   {
                    switch (dir)
                    {
                    case Generic::Direction::DEFAULT_A: return os << "default a";
                    case Generic::Direction::DEFAULT_B: return os << "default b";
                    default: return os << "unknown generic direction";
                    }
                   }

                   else
                   {
                       return os << "unknown direction variant type";
                   } }, direction);
}

///////////////////////
// TRAINLINE HELPERS //
///////////////////////

inline bool trainlines_equal(const TrainLine &a, const TrainLine &b)
{
    if (a.index() != b.index())
    {
        return false;
    }

    return std::visit([&](const auto &lhs, const auto &rhs) -> bool
                      {
                        using T = std::decay_t<decltype(lhs)>;
                        using U = std::decay_t<decltype(rhs)>;
                        if constexpr (std::is_same_v<T, U>){
                            return lhs == rhs;
                        } else {
                            return false;
                        } }, a, b);
}

inline TrainLine trainline_from_string(const std::string &str)
{
    static const std::unordered_map<std::string, TrainLine> trainline_map{
        {"1", SUB::TrainLine::ONE},
        {"2", SUB::TrainLine::TWO},
        {"3", SUB::TrainLine::THREE},
        {"4", SUB::TrainLine::FOUR},
        {"5", SUB::TrainLine::FIVE},
        {"6", SUB::TrainLine::SIX},
        {"7", SUB::TrainLine::SEVEN},
        {"A", SUB::TrainLine::A},
        {"C", SUB::TrainLine::C},
        {"E", SUB::TrainLine::E},
        {"N", SUB::TrainLine::N},
        {"Q", SUB::TrainLine::Q},
        {"R", SUB::TrainLine::R},
        {"W", SUB::TrainLine::W},
        {"B", SUB::TrainLine::B},
        {"D", SUB::TrainLine::D},
        {"F", SUB::TrainLine::F},
        {"M", SUB::TrainLine::M},
        {"G", SUB::TrainLine::G},
        {"L", SUB::TrainLine::L},
        {"J", SUB::TrainLine::J},
        {"Z", SUB::TrainLine::Z},
        {"GS", SUB::TrainLine::GS},
        {"FS", SUB::TrainLine::FS},
        {"H", SUB::TrainLine::H},
        {"SI", SUB::TrainLine::SI},
        {"SIR", SUB::TrainLine::SI},
        {"Harlem", MNR::TrainLine::HARLEM},
        {"Hudson", MNR::TrainLine::HUDSON},
        {"New Haven", MNR::TrainLine::NEW_HAVEN},
        {"New Canaan", MNR::TrainLine::NEW_HAVEN},
        {"Waterbury", MNR::TrainLine::NEW_HAVEN},
        {"Danbury", MNR::TrainLine::NEW_HAVEN},
        {"Babylon Branch", LIRR::TrainLine::BABYLON},
        {"Far Rockaway Branch", LIRR::TrainLine::FAR_ROCKAWAY},
        {"Hempstead Branch", LIRR::TrainLine::HEMPSTEAD},
        {"Long Beach Branch", LIRR::TrainLine::LONG_BEACH},
        {"Montauk Branch", LIRR::TrainLine::MONTAUK},
        {"Oyster Bay Branch", LIRR::TrainLine::OYSTER_BAY},
        {"Port Jefferson Branch", LIRR::TrainLine::PORT_JEFFERSON},
        {"Port Washington Branch", LIRR::TrainLine::PORT_WASHINGTON},
        {"Ronkonkoma Branch", LIRR::TrainLine::RONKONKOMA},
        {"West Hempstead Branch", LIRR::TrainLine::WEST_HEMPSTEAD},
    };

    auto it = trainline_map.find(str);
    if (it != trainline_map.end())
    {
        return it->second;
    }
    else
    {
        return Generic::TrainLine::DEFAULT;
    }
}

inline std::ostream &operator<<(std::ostream &os, const TrainLine &trainline)
{
    return std::visit([&os](const auto &line) -> std::ostream &
                      {
                   using T = std::decay_t<decltype(line)>;
                   if constexpr (std::is_same_v<T, SUB::TrainLine>)
                   {
                       switch (line)
                       {
                        case SUB::TrainLine::ONE: return os << "1";
                        case SUB::TrainLine::TWO: return os << "2";
                        case SUB::TrainLine::THREE: return os << "3";
                        case SUB::TrainLine::FOUR: return os << "4";
                        case SUB::TrainLine::FIVE: return os << "5";
                        case SUB::TrainLine::SIX: return os << "6";
                        case SUB::TrainLine::SEVEN: return os << "7";
                        case SUB::TrainLine::A: return os << "A";
                        case SUB::TrainLine::C: return os << "C";
                        case SUB::TrainLine::E: return os << "E";
                        case SUB::TrainLine::N: return os << "N";
                        case SUB::TrainLine::Q: return os << "Q";
                        case SUB::TrainLine::R: return os << "R";
                        case SUB::TrainLine::W: return os << "W";
                        case SUB::TrainLine::B: return os << "B";
                        case SUB::TrainLine::D: return os << "D";
                        case SUB::TrainLine::F: return os << "F";
                        case SUB::TrainLine::M: return os << "M";
                        case SUB::TrainLine::G: return os << "G";
                        case SUB::TrainLine::L: return os << "L";
                        case SUB::TrainLine::J: return os << "J";
                        case SUB::TrainLine::Z: return os << "Z";
                        case SUB::TrainLine::GS: return os << "42nd Street Shuttle";
                        case SUB::TrainLine::FS: return os << "Franklin Street Shuttle";
                        case SUB::TrainLine::H: return os << "Far Rockaway Shuttle";
                        case SUB::TrainLine::SI: return os << "Station Island Railway";
                        default: return os << "unknown subway train line";
                       }
                   }

                   else if constexpr (std::is_same_v<T, MNR::TrainLine>)
                   {
                       switch (line)
                       {
                        case MNR::TrainLine::HARLEM: return os << "Harlem";
                        case MNR::TrainLine::HUDSON: return os << "Hudson";
                        case MNR::TrainLine::NEW_HAVEN: return os << "New Haven";
                        default: return os << "unknown metro north train line";
                       }
                   }

                   else if constexpr (std::is_same_v<T, LIRR::TrainLine>)
                   {
                       switch (line)
                       {
                       case LIRR::TrainLine::BABYLON: return os << "Babylon";
                       case LIRR::TrainLine::FAR_ROCKAWAY: return os << "Far Rockaway";
                       case LIRR::TrainLine::HEMPSTEAD: return os << "Hempstead";
                       case LIRR::TrainLine::LONG_BEACH: return os << "Long Beach";
                       case LIRR::TrainLine::MONTAUK: return os << "Montauk";
                       case LIRR::TrainLine::OYSTER_BAY: return os << "Oyster Bay";
                       case LIRR::TrainLine::PORT_JEFFERSON: return os << "Port Jefferson";
                       case LIRR::TrainLine::PORT_WASHINGTON: return os << "Port Washington";
                       case LIRR::TrainLine::RONKONKOMA: return os << "Ronkonkoma";
                       case LIRR::TrainLine::WEST_HEMPSTEAD: return os << "West Hempstead";
                       default: return os << "unknown long island railroad train line";
                       }
                   }

                   else if constexpr (std::is_same_v<T, Generic::TrainLine>)
                   {
                    switch (line)
                    {
                        case Generic::TrainLine::DEFAULT: return os << "Generic Default";
                        default: return os << "unknown generic train line";
                    }
                   }

                   else
                   {
                       return os << "unknown train line variant type";
                   } },
                      trainline);
}


inline std::string trainline_to_string(const TrainLine &trainline)
{
    std::ostringstream oss;
    oss << trainline;
    return oss.str();
}

inline std::string direction_to_string(const Direction& direction)
{
    std::ostringstream oss;
    oss << direction;
    return oss.str();
}