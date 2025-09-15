# Metro North 

## Overview

The `MetroNorth`, located in the `Transit::Map` namespace, is a singleton class derived from [`Graph`](/docs/map/graph.md) that models the Metro North Railroad transit system. It loads station and connection data from MTA sources to build an internal graph representation of the Metro North network.

## Responsibilites

- Parses MTA data files to construct nodes (stations) an weighted edges (connection)

## Methods

For full details, see the [header](/include/map/metro_north.h) and [source](/src/map/metro_north.cpp) files

### Constructor

- `MetroNorth()` : private constructor implementing the singleton pattern.

### Public

- `get_instance()` : returns the singleton instance.

### Private

- `load_stations(...)` : loads station data and creates graph nodes.

- `load_connections(...)` : loads route data and creates weighted edges; also drives `Route` creation.

- `merge_segments(...)` : merges multiple ordered segments into a single sequence, while preserving relative order.

- `k_way_merge(...)` : merging helper that utilizes precedence constraints.

- `handle_branches(...)` : return branch portion of route sequence.

- `get_branch_point(...)` : returns int id of branch point.

- `get_branch_point_name(...)` : returns string name of branch point.

## Dependencies

The `MetroNorth` class is derived from [`Graph`](/docs/map/graph.md)

- Uses:
  - `Utils::open_and_parse(...)` to open files, validate required columns and invokes a callback per line
  - `Utils::split(...)` to tokenize strings on delimiters
  - `Utils::from_tokens(...)` to map tokenized row into values
  - C++ file I/O and string handling
  - K-way merging for constructing `Route` sequences

- For use in:
  - Main application

## Example Usage
```cpp
// get singleton instance of MetroNorth
Transit::Map::MetroNorth &mnr {Transit::Map::MetroNorth::get_instance()};

// retrieve all train line routes and their stop sequences
std::unordered_map<TrainLine, std::vector<Route>> routes { mnr.get_routes() };
```

## Notes

### Design Decisions

- The `MetroNorth` class is implemented as a singleton to ensure consistency across the simulation and avoid redundant memory usage and initialization overhead.

- The `MetroNorth` class extends the [`Graph`](graph.md) class to inherit core pathfinding and adjacency logic and layers in Metro North specific metadata and methods.

- The `MetroNorth` class reads from files preprocessed by the [`data pipeline`](/data_pipeline/DATA_PIPELINE.md), significantly reducing runtime parsing and file I/O overhead.

- The `MetroNorth` class utilizes k-way merging to construct complete routes from fragmented input, providing a more accurate representation the transit system. For details on Metro North specific preprocessing normalizations, see the [`data pipeline`](/data_pipeline/DATA_PIPELINE.md).

### Future Improvements

- Future versions will also include graph serialization after initial load for increased performance.