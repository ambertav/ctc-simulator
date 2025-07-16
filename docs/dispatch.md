# Dispatch

## Overview

The `Dispatch` controls the simulation loop. For each tick, it updates train positions, manages signal state, and ensures schedule compliance.

## Responsibilities
- Authorizes trains that request movement into a signal
- Manages signal state changes
- Orchestrates the logging of all simulation runtime events (e.g. signal changes, train arrivals, and train departures)

## Methods

For full details, see the [header](/include/core/dispatch.h) and [source](/src/core/dispatch.cpp) files

### Constructor
- `Dispatch(...)` : constructs the `Dispatch` instance.

### Public

- `get_schedule()` : returns a `std::unordered_map` of station id keys and corresponding `EventQueues` values.
  
- `load_schedule(...)` : reads from schedule file and constructs an `EventQueues` object for each station within the schedule.
  
- `update(...)` : authorizes train movement if applicable and invokes the logging of all simulation events.

### Private
- `authorize(...)` : manages if signals can be changes and authorizes the requesting train to move.
  
- `handle_spawns(...)` : iterates through yard ids and manages the dispatch of trains according to schedule.
  
- `spawn_train(...)` : dispatches train from yard.
  
- `despawn_train(...)` : returns train to yard after completing the route.

## Dependencies

The [`Scheduler`](scheduler.md) class constructs the schedule to be loaded by the `Dispatch`, which constructs `EventQueues` containing arrival and departure priority queues for each station.

The [`Factory`](factory.md) class is responsible for creating and initializing all simulation components. The components are accessed via getter methods and passed into the `Dispatch` constructor. 

- Uses:
  - [`Train`](/include//core/train.h), [`Signal`](/include/core/signal.h), [`Track`](/include/core/track.h), [`Platform`](/include/core/platform.h), [`Station`](/include/core/station.h), [`Logger`](/include/systems/logger.h)

- For use in:
  - Main application

## Example Usage
```cpp
// core simulation classes for Dispatch constructor
std::vector<Station*> stations { factory.get_stations() };
std::vector<Train*> trains { factory.get_trains() };
std::vector<Track*> tracks { factory.get_tracks() };
std::vector<Platform*> platforms { factory.get_platforms() };
std::vector<Signal*> signals { factory.get_signals() };

// instantiate Logger for simulation output
Logger logger{"/logs/sim.txt"};

// instantiate Dispatch
Dispatch dispatch(stations, trains, tracks, platforms, signals, logger);

// load schedule (e.g. data/schedule.csv)
dispatch.load_schedule("data/schedule.csv");

// run simulation loop for N ticks
for (int i = 0; i < N; i++)
{
    dispatch.update(i);
}
```

## Notes

### Design Decisions

- Station-specific priority queues (i.e., `EventQueues`) for both arrivals and departures were chosen to manage event ordering and processing at each station independently for accurate logging and schedule adherence. Each station's `EventQueues` is accessible in O(1) with the `Station` id.

- `Dispatch` holds vectors of raw pointers to objects to decouple ownership and adhere to the factory design pattern. The [`Factory`](factory.md) collaborates with the [`Scheduler`](scheduler.md) to create all necessary simulation objects. Thus, the `Dispatch` is solely responsible for managing centralized traffic control within the simulation.

### Future Improvements
- The `Dispatch` class will be updated to support switch changes and handling randomized delays.
