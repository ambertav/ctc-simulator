# Long Island Railroad 

## Overview

The `LongIslandRailroad`, located in the `Transit::Map` namespace, is a singleton class derived from [`Graph`](/docs/map/graph.md) that models the Long Island Railroad transit system. It loads station and connection data from MTA sources to build an internal graph representation of the Long Island Railroad network.

## Responsibilites

- Parses MTA data files to construct nodes (stations) an weighted edges (connection)

## Methods

For full details, see the [header](/include/map/lirr.h) and [source](/src/map/lirr.cpp) files

### Constructor

- `LongIslandRailroad()` : private constructor implementing the singleton pattern.

### Public

- `get_instance()` : returns the singleton instance.

### Private

- `load_stations(...)` : loads station data and creates graph nodes.

- `load_connections(...)` : loads route data and creates weighted edges; also drives `Route` creation.

- `merge_segments(...)` : merges multiple ordered segments into a single sequence, while preserving relative order.

- `k_way_merge(...)` : merging helper that utilizes precedence constraints.

## Dependencies

The `LongIslandRailroad` class is derived from [`Graph`](/docs/map/graph.md)

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
// get singleton instance of LongIslandRailroad
Transit::Map::LongIslandRailroad &lirr {Transit::Map::LongIslandRailroad::get_instance()};

// retrieve all train line routes and their stop sequences
std::unordered_map<TrainLine, std::vector<Route>> routes { mnr.get_routes() };
```

## Notes

### Design Decisions

- The `LongIslandRailroad` class is implemented as a singleton to ensure consistency across the simulation and avoid redundant memory usage and initialization overhead.

- The `LongIslandRailroad` class extends the [`Graph`](graph.md) class to inherit core pathfinding and adjacency logic and layers in Metro North specific metadata and methods.

- The `LongIslandRailroad` class reads from files preprocessed by the [`data pipeline`](/data_pipeline/DATA_PIPELINE.md), significantly reducing runtime parsing and file I/O overhead.

- The `LongIslandRailroad` class utilizes k-way merging to construct complete routes from fragmented input, providing a more accurate representation the transit system. For details on Long Island Railroad specific preprocessing normalizations, see the [`data pipeline`](/data_pipeline/DATA_PIPELINE.md).

### Future Improvements

- Future versions will also include graph serialization after initial load for increased performance.