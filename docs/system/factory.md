# Factory

## Overview

The `Factory` creates and owns all simulation objects for each rail system using `std::unique_ptr`, managing their lifetimes automatically. The `Factory` is owned and managed by [`CentralControl`](/docs/central_control.md). It builds the objects based on the `Route` for each `TrainLine`, obtained from [`Graph`](graph.md) class and its derived classes (e.g., [`Subway`](/docs/map/subway.md), [`MetroNorth`](/docs/map/metro_north.md), [`LongIslandRailroad`](/docs/map/lirr.md)) located within the `Transit::Map` namespace.


## Responsibilities

- Creates all trains, stations, signals, platforms, tracks, and switches for the rail system for use in the simulation
- Manages the lifetime of simulation objects
- Returns raw pointers to the objects for use in [`Dispatch`](/docs/dispatch.md)

## Methods

For full details, see the [header](/include/system/factory.h) and [source](/src/system/factory.cpp) files

### Constructor
- `Factory()` : default constructor that creates the `Factory` instance.

### Public

- `build_network(...)` : drives simulation resource creation and connects the network according to routes.
  
- `get_trains(...)` : returns vector of raw pointers for trains, overloaded to return trains by train line.
  
- `get_stations(...)` : returns vectory of raw pointers for stations, overloaded to return trains by train line.
  
- `get_signals()` : returns vector of raw pointers for signals.
  
- `get_platforms()` : returns vector of raw pointers for platforms.
  
- `get_tracks()` : returns vector of raw pointers for tracks.

- `get_switches()` : returns vector of raw pointers for switches.

### Private

- `generate_signal_id()` : increments and returns the next signal id.

- `generate_track_id()` : increments and returns the next track id.

- `generate_switch_id()` : increments and returns the next switch id.

- `create_trains(...)` : creates the trains specified by the registry.
  
- `create_stations(...)` : creates stations and corresponding platforms according to the adjacency list; also creates the yards specified by the registry.

- `create_track(...)` : creates a track in between two platforms and drives switch creation.

- `create_switch(...)` : creates a switch between a platform and track if applicable.

## Dependencies

The [`Registry`](/docs/system/registry.md) class provides standardized infromation about trains and yards using encoded int ids. The decoded data includes `TrainLine` and `Direction` details. The [`Registry`](/docs/system/registry.md) ensures consistency between the `Factory` and the [`Scheduler`](/docs/system/scheduler.md).

The [`Graph`](/docs/map/graph.md) class and its derived classes (e.g., [`Subway`](/docs/map/subway.md), [`MetroNorth`](/docs/map/metro_north.md), [`LongIslandRailroad`](/docs/map/lirr.md)), which reside within the `Transit::Map` namespace, provide the adjacency list and `TrainLine` specific `Route`. These are then iterated over by the `Factory` to create the corresponding stations, platforms, signals, switches, and connecting tracks.

The [`CentralControl`](/docs/core/central_control.md) class owns and manages the `Factory`.

- Uses:
  - `Registry::get_train_registry(...)` to provide train information
  - `Registry::get_yard_registry(...)` to provide yard information and connectivity
  - `Registry::decode(...)` to extract information
  - `Graph`, which provides the adjacency list for station creation and the `Route` as source of truth for network construction

- For use in:
  - [`CentralControl`](/docs/core/central_control.md)

## Example Usage
```cpp
// NOTE: the factory class is not intended to be used directly within the simulation
Factory factory{};
factory.build_network(graph, registry, system_code);
```

## Notes

### Design Decisions
- The `Factory` is owned by [`CentralControl`](/docs/core/central_control.md) to encapsulate all simulation resources for a rail system in a centralized location.

- The `Factory` owns all simulation objects via `std::unique_ptr` to ensure clear ownership semantics and automatic memory management.

- Simulation objects are exposed externally as raw pointers via getters to decouple ownership from usage, following the factory design pattern.

- The `Factory` relies on the [`Registry`](/docs/system/registry.md) to standardize train and yard information, ensuring data consistency between the `Factory` and the [`Scheduler`](/docs/system/scheduler.md).

- Simulation objects are created based on the routes and adjacency data from the `Graph` and derived classes, ensuring connections are representative of the underlying transit map.