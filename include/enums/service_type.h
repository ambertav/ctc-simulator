#pragma once

#include <iostream>

enum class ServiceType
{
    LOCAL,
    EXPRESS
};

inline std::ostream &operator<<(std::ostream &os, ServiceType type)
{
    switch (type)
    {
    case ServiceType::LOCAL:
        return os << "local";
    case ServiceType::EXPRESS:
        return os << "express";
    }
}