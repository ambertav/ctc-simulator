# Scheduler

## Overview

The `Scheduler` generates a simulation schedule based on the [`Graph`](graph.md) class and its derived classes (e.g., [`Subway`](/docs/map/subway.md), [`MetroNorth`](/docs/map/metro_north.md), [`LongIslandRailroad`](/docs/map/lirr.md)). It creates arrival and deparature events at each station along the route, timestamped using simulation ticks. The `Scheduler` uses train and yard object information obtained from the [`Registry`](/docs/system/registry.md). The schedule is then written to a JSON file specific for each rail system for use during the simulation. 

## Responsibilities

- Determines a bidirectional schedule for a given transit system
- Writes the schedule out to a JSON file for use in the simulation loop by [`Dispatch`](/docs/core/dispatch.md)

## Methods

For full details, see the [header](/include/systems/scheduler.h) and [source](/src/systems/scheduler.cpp) files

### Constructor

- `Scheduler()` : default constructor that creates the `Scheduler` instance.

### Public

- `write_schedule(...)` : processes the rail system and writes the schedule out to the specified output JSON.

### Private

- `process_system(...)` : matches route and yards to each train and drives the schedule generation for that train.

- `generate_train_schedule(...)` : generates schedule for a train, computing arrival and departure ticks at each station and yard.

## Dependencies

The [`Registry`](/docs/system/registry.md) class provides standardized information about trains and yards using encoded int ids. The decoded data includes `TrainLine` and `Direction` details. The [`Registry`](/docs/system/registry.md) ensures consistency between the [`Factory`](/docs/system/factory.md) and the `Scheduler`.

The [`Graph`](/docs/map/graph.md) class and its derived classes (e.g., [`Subway`](/docs/map/subway.md), [`MetroNorth`](/docs/map/metro_north.md), [`LongIslandRailroad`](/docs/map/lirr.md)), which reside within the `Transit::Map` namespace, provide the `TrainLine` specific `Route`. The `Route` is then processed by the `Scheduler` to generate the simulation schedule for each rail system.

- Uses
  - `Registry::get_train_registry(...)` to provide train information
  - `Registry::get_yard_registry(...)` to provide yard information and connectivity
  - `Registry::decode(...)` to extract information
  - `Graph`, which provides the `Route` that defines the stop order in scheduling
  - `<nlohmann/json.hpp>` to facilitate JSON formatting and parsing

- For use in:
  - Main application

## Example Usage
```cpp
// instantiate the Scheduler
Scheduler scheduler{};

// concurrent schedule generation
std::vector<std::future<void>> futures{};
futures.emplace_back(std::async(std::launch::async, [&]{ 
    scheduler.write_schedule(subway, registry, subway_dir, Constants::System::SUBWAY); 
}));
futures.emplace_back(std::async(std::launch::async, [&]{ 
    scheduler.write_schedule(mnr, registry, mnr_dir, Constants::System::METRO_NORTH); 
}));
futures.emplace_back(std::async(std::launch::async, [&]{ 
    scheduler.write_schedule(lirr, registry, lirr_dir, Constants::System::LIRR); 
}));

for (auto &f : futures)
{
    f.wait();
}
```

## Notes

### Design Decisions

- The `Scheduler` is intentionally decoupled from the simulation runtime to ensure that the simulation operates with a valid schedule.

- The `Scheduler` is stateless and can generate schedules for multiple rail systems concurrently, as it outputs each rail system's schedule to separate files. This design enables parallel processing without shared state between rail systems.

- The `Scheduler` is outputs the schedule in JSON format grouped by `TrainLine` and trains for enhanced readibility by the [`Dispatch`](/docs/core/dispatch.md).

- The `Scheduler` relies on the [`Registry`](/docs/system/registry.md) to standardize train and yard information, ensuring data consistency between the `Scheduler` and the [`Factory`](/docs/system/factory.md).

- The `Constants` namespace provides simulation configuration values used by the `Scheduler`, such as `STATION_DWELL_TIME`, `DEFAULT_TRAVEL_TIME`, and `YARD_DEPARTURE_GAP`.