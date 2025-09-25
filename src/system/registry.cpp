/**
 * for details on design, see:
 * docs/system/registry.md
 */

#include "system/registry.h"

Registry::Registry()
{
    build_registry();
}

const std::vector<int> &Registry::get_train_registry(Constants::System system_code) const
{
    auto it{train_registry.find(system_code)};
    if (it == train_registry.end())
    {
        return {};
    }
    else
    {
        return it->second;
    }
}

const std::vector<std::pair<int, int>> &Registry::get_yard_registry(Constants::System system_code) const
{
    auto it{yard_registry.find(system_code)};
    if (it == yard_registry.end())
    {
        return {};
    }
    else
    {
        return it->second;
    }
}

//[ system (4 bits) | train_line_code (8 bits) | direction_code (12 bits) | order (8 bits) ]
int Registry::encode(Constants::System system_code, int train_line_code, int direction_code, int instance)
{
    int code {static_cast<int>(system_code)};
    return ((code & 0xF) << 28) | ((train_line_code & 0xFF) << 20) | ((direction_code & 0xFFF) << 8) | (instance & 0xFF);
}

Info Registry::decode(int encoded_id) const
{
    int code{(encoded_id >> 28) & 0xF};
    int train_line_code{(encoded_id >> 20) & 0xFF};
    int direction_code{(encoded_id >> 8) & 0xFFF};
    int instance{encoded_id & 0xFF};

    TrainLine train_line{};
    Direction direction{};

    Constants::System system_code{static_cast<Constants::System>(code)};

    switch (system_code)
    {
    case Constants::System::SUBWAY:
    {
        train_line = static_cast<SUB::TrainLine>(train_line_code);
        direction = static_cast<SUB::Direction>(direction_code);
        break;
    }
    case Constants::System::METRO_NORTH:
    {
        train_line = static_cast<MNR::TrainLine>(train_line_code);
        direction = static_cast<MNR::Direction>(direction_code);
        break;
    }
    case Constants::System::LIRR:
    {
        train_line = static_cast<LIRR::TrainLine>(train_line_code);
        direction = static_cast<LIRR::Direction>(direction_code);
        break;
    }
    default:
    {
        throw std::invalid_argument("Failed to decode " + std::to_string(encoded_id) + ". Invalid system code: " + std::to_string(static_cast<int>(system_code)));
    }
    }

    return {encoded_id, system_code, train_line, direction, instance};
}

void Registry::build_registry()
{
    for (const auto &[name, system] : Constants::SYSTEMS)
    {
        int tl_count{};
        int dir_count{};

        switch (system)
        {
        case Constants::System::SUBWAY:
        {
            tl_count = static_cast<int>(SUB::TrainLine::COUNT);
            dir_count = static_cast<int>(SUB::Direction::COUNT);
            break;
        }
        case Constants::System::METRO_NORTH:
        {
            tl_count = static_cast<int>(MNR::TrainLine::COUNT);
            dir_count = static_cast<int>(MNR::Direction::COUNT);
            break;
        }
        case Constants::System::LIRR:
        {
            tl_count = static_cast<int>(LIRR::TrainLine::COUNT);
            dir_count = static_cast<int>(LIRR::Direction::COUNT);
            break;
        }
        default:
        {
            throw std::invalid_argument("Invalid system code: " + std::to_string(static_cast<int>(system)) + ", " + std::string(name));
        }
        }

        generate_trains(system, tl_count, dir_count);
        generate_yards(system, tl_count);
    }
}

void Registry::generate_trains(Constants::System system_code, int tl_count, int dir_count)
{
    int trains_per_dir{Constants::DEFAULT_TRAINS_PER_LINE / 2};
    int total_trains{tl_count * dir_count * trains_per_dir};

    train_registry[system_code].reserve(total_trains);

    for (int i{0}; i < total_trains; ++i)
    {
        train_registry[system_code].push_back(encode(
            system_code,
            i / (dir_count * trains_per_dir) /* TrainLine enumertion */,
            (i / trains_per_dir) % dir_count /* Direction enumeration */,
            i % trains_per_dir /* instance */
            ));
    }
}

void Registry::generate_yards(Constants::System system_code, int tl_count)
{
    yard_registry[system_code].reserve(tl_count);

    auto directions{Constants::get_directions_by_system_code(system_code)};
    int dir_one{std::visit([](auto d) { return static_cast<int>(d); }, directions[0])};
    int dir_two{std::visit([](auto d) { return static_cast<int>(d); }, directions[1])};

    for (int i{0}; i < tl_count; ++i)
    {
        yard_registry[system_code].emplace_back(
            encode(system_code, i /* TrainLine enumeration */, dir_one /* Direction enumeration */, 0 /* instance */),
            encode(system_code, i /* TrainLine enumeration */, dir_two /* Direction enumeration */, 0 /* instance */));
    }
}