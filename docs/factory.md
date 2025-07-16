# Factory

## Overview

The `Factory` creates and owns all simulation objects using `std::unique_ptr`, managing their lifetimes automatically. It builds the objects based on the route's `Path`, which is obained from the [`Graph`](graph.md) class and its derived classes (e.g, [`Subway`](subway.md)) located within the `Transit::Map` namespace.


## Responsibilities
- Creates all stations, platforms, signals, tracks, and trains for the simulation
- Manages the lifetime of simulation objects
- Returns raw pointers to the objects for use in [`Dispatch`](/docs/dispatch.md)

## Methods

For full details, see the [header](/include/core/factory.h) and [source](/src/core/factory.cpp) files

### Constructor
- `Factory()` : default constructor that creates the `Factory` instance.

### Public

- `build_network(...)` : creates simulation objects and connects stations, platforms, and tracks to create the route.
  
- `get_trains()` : returns `std::vector` of raw pointers for trains.
  
- `get_stations()` : returns `std::vector` of raw pointers for stations.
  
- `get_signals()` : returns `std::vector` of raw pointers for signals.
  
- `get_platforms()` : returns `std::vector` of raw pointers for platforms.
  
- `get_tracks()` : returns `std::vector` of raw pointers for tracks.
  
- `clear()` : clears simulation objects.

### Private

- `create_trains(...)` : creates the total amount of trains for a particular `TrainLine`.
  
- `create_stations(...)` : creates the stations represented in `path.nodes` and returns the start and ending yard ids.

- `create_network(...)` : uses the `Path` and the start and ending yard ids to build and connect the routes for `Direction::UPTOWN` and `Direction::DOWNTOWN`.

## Dependencies

The [`Graph`](graph.md) class and its derived classes (e.g [`Subway`](subway.md)), which reside within the `Transit::Map` namespace, construct the [`Path`](/include/map/graph.h). The `Path` is then iterated over by the `Factory` to create the corresponding stations, platforms, signals, and connecting tracks.

- Uses:
  - `Path`, which holds the `Node` stations in route order and distance weights between nodes.

- For use in:
  - Main application

## Example Usage
```cpp
// instantiate Factory
Factory factory;

// build objects and connect route
factory.build_network(number_of_trains, path);
```

## Notes

### Design Decisions
- The `Factory` owns all simulation objects via `std::unique_ptr` to ensure clear ownership semantics and automatic memory management.

- Simulation objects are exposed externally as raw pointers via getters to decouple owenship from usage, following the factory design pattern.

- The simulation network is based on the route's `Path` from the [`Graph`](graph.md) class hierarchy, ensuring the stations and connections are consistent with the underlying transit map.

### Future Improvements
- The `Factory` class will be updated to introduce multithreading, enabling parallel object creation and network building to facilitate future simulations running in concurrent threads to emulate an entire transit system.