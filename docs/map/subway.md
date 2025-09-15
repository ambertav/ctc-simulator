# Subway 

## Overview

The `Subway`, located in the `Transit::Map` namespace, is a singleton class derived from [`Graph`](/docs/map/graph.md) that models the New York City Subway transit system. It loads station and connection data from MTA sources to build an internal graph representation of the subway network.

## Responsibilites

- Parses MTA data files to construct nodes (stations) an weighted edges (connection)

## Methods

For full details, see the [header](/include/map/subway.h) and [source](/src/map/subway.cpp) files

### Constructor

- `Subway()` : private constructor implementing the singleton pattern.

### Public

- `get_instance()` : returns the singleton instance.

### Private

- `load_stations(...)` : loads station data and creates graph nodes.

- `load_connections(...)` : loads route data and creates weighted edges; also drives `Route` creation.

## Dependencies

The `Subway` class is derived from [`Graph`](/docs/map/graph.md)

- Uses:
  - `Utils::open_and_parse(...)` to open files, validate required columns and invokes a callback per line
  - `Utils::split(...)` to tokenize strings on delimiters
  - `Utils::from_tokens(...)` to map tokenized row into values
  - C++ file I/O and string handling

- For use in:
  - Main application

## Example Usage
```cpp
// get singleton instance of Subway
Transit::Map::Subway &subway {Transit::Map::Subway::get_instance()};

// retrieve all train line routes and their stop sequences
std::unordered_map<TrainLine, std::vector<Route>> routes { subway.get_routes() };
```

## Notes

### Design Decisions

- The `Subway` class is implemented as a singleton to ensure consistency across the simulation and avoid redundant memory usage and initialization overhead.

- The `Subway` class extends the [`Graph`](graph.md) class to inherit core pathfinding and adjacency logic and layers in subway-specific metadata and methods.

- The `Subway` class reads from files preprocessed by the [`data pipeline`](/data_pipeline/DATA_PIPELINE.md), significantly reducing runtime parsing and file I/O overhead.

- The `Subway` station connections and routes were refined to ensure that each `Route` and subsequent edges adhered to typical service stop patterns. 

### Future Improvements

- Future versions will also include graph serialization after initial load for increased performance.