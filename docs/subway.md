# Subway 

## Overview

The `Subway`, located in the `Transit::Map` namespace, is a singleton class derived from [`Graph`](graph.md) that models the New York City Subway transit system. It loads station and connection data from MTA sources to build an internal graph representation of the subway network.

## Responsibilites
- Parses MTA data files to construct nodes (stations) an weighted edges (connection)
- Provides pathfinding and route information for simulation analysis.
- Provides full bidirectional `Route` information, containing the full stop sequence along all possible trip headsigns for that train line.

## Methods

For full details, see the [header](/include/map/subway.h) and [source](/src/map/subway.cpp) files

### Constructor

- `Subway()` : private constructor implementing the singleton pattern.

### Public

- `get_instance()` : returns the singleton instance.

- `get_routes()` : retrieves all train line routes with respective stop sequences.

### Private
- `load_stations(...)` : loads station data from files, creating graph nodes.

- `load_connections(...)` : loads connection data between stations, creating weighted edges; also creates a `std::vector` of `Route` per train line and trip headsign.

- `add_route(...)` : constructs a `Route` for a specific train line using trip headsign and stop sequence.

- `extract_headsign_to_trip(...)` : parses MTA data to group trip headsigns, route ids, and directions, mapping them to specific trip ids.

- `extract_trip_to_stops(...)` : parses MTA data to determine station seuqences for each trip id and orchestrates `Route` creation.

- `open_and_parse(...)` : helper method that opens a file and parses its contents line by line.

## Dependencies

The `Subway` class is derived from [`Graph`](graph.md)

- Uses:
  - `Utils::split(...)` to tokenize strings on delimiters
  - C++ file I/O and string handling

- For use in:
  - Main application

## Example Usage
```cpp
// get singleton instance of Subway
Transit::Map::Subway &subway = Transit::Map::Subway::get_instance();

// find path between two stations using their string ids
Transit::Map::Path path { subway.find_path("100", "101") };

// retrieve all train line routes and their stop sequences
std::unordered_map<TrainLine, std::vector<Route>> routes { subway.get_routes() };
```

## Notes

### Design Decisions

- The `Subway` class is implemented as a singleton to ensure consistency across the simulation and avoid redundant memory usage and initialization overhead.

- The `Subway` class extends the [`Graph`](graph.md) class to inherit core pathfinding and adjacency logic and layers in subway-spectific metadata and methods.

- Currently, the `Subway` class reads directly from raw GTFS files, which provides flexibility but incurs parsing and file I/O overhead during each simulation run.

### Future Improvements

- The `Subway` class will be updated to support preprocessed and indexed input files, reducing runtime file I/O and parsing overhead. The preprocessing pipeline will extract only the data relevant to the simulation.

- Future versions will also include graph serialization after initial load for better performance.