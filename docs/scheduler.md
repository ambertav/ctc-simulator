# Scheduler

## Overview

The `Scheduler` generates a simulation schedule based on a the given route `Path`. It creates arrival and deparature events at each station along the route, timestamped using simulation ticks. These events are generated for both directions and all train objects, and are written to a CSV file (e.g., `data/schedule.csv`) for use during the simulation. 

For details about the generated schedule files, see the [data folder documentation](data/DATA.md)

## Responsibilities
- Determines a bidirectional schedule for a given path
- Writes the schedule out to a CSV file for use in the simulation loop by `Dispatch`

## Methods

For full details, see the [header](/include/systems/scheduler.h) and [source](/src/systems/scheduler.cpp) files

### Constructor
- `Scheduler(...)` : constructs the `Scheduler` instance with a specified output CSV and optional configuration. 

### Public
- `create_schedule(...)` : processes the given path and writes the schedule out to the specified output CSV.

## Dependencies

The [`Graph`](graph.md) class and its derived classes (e.g [`Subway`](subway.md)), which reside within the `Transit::Map` namespace, construct the [`Path`](/include/map/graph.h). The `Path` is then processed by the `Scheduler` to generate the simulation schedule.

- Uses
  - `Path`, `Node` from the `Transit::Map` namespace
  - C++ file I/O and string handling

- For use in:
  - Main application

## Example Usage
```cpp
// instantiate the Scheduler with default configuration
Scheduler default_scheduler{"data/schedule.csv"};

// OR instantiate the Scheduler with custom configuration
int time_between_train_spawns { 3 };    // in simulation ticks
int dwell_time_at_stations { 2 };       // in simulation ticks
int number_of_trains { 6 };

Scheduler custom_scheduler{
    "data/schedule.csv",
    time_between_train_spawns, 
    dwell_time_at_stations, 
    number_of_trains
};

// generate and write schedule
default_scheduler.create_schedule(path);
custom_scheduler.create_schedule(path);
```

## Notes

### Design Decisions

- The `Scheduler` is intentionally decoupled from the simulation runtime to ensure that the simulation operates with a valid schedule.

- The schedule is wrirrten in CSV format for readibility and debugging.

- The `spawn_train_gap` attribute (time between train spawns) facilitates enforcing safe spacing between consecutive trains.

### Future Improvements

- The `Scheduler` class will be updated to introduce multithreaded schedule generation for each train route in preparation to simulate an entire transit system in parallel.