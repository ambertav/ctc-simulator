# Graph

## Overview

The `Graph`, located in the `Transit::Map` namespace, is a weighted, undirected graph that provides the core functionality for constructing a transit map. It is designed to be extended by derived classes that model real-world transit systems (e.g., [`Subway`](subway.md)).

## Responsibilities
- Provides core methods for adding/removing nodes, managing weighted edges, and pathfinding.
- Remains flexible for extension by any class emulating real world transit maps

## Methods

For full details, see the [header](/include/map/graph.h) and [source](/src/map/graph.cpp) files

### Constructor
- `Graph()` : default constructor that creates the `Graph` instance.

### Public
- `add_node(...)` : constructs node and adds to graph.

- `remove_node(...)` : removes node, overloaded to accept pointer and string id.

- `add_edge(...)` : adds a weighted, undirected edge between two nodes, overloaded to accept pointers and string ids.

- `remove_edge(...)` : removes an edge between two node pointers.

- `update_node(...)` : updates node to add to the `TrainLine` and `gtfs_ids` attributes.
  
- `get_node(...)` : returns a raw pointer to the node

- `get_adjacency_list()` : returns the entire adjacency list.

- `print()` : prints the graph's adjacency list to the console.

- `find_path(...)` : accepts two string ids and finds a path between the corresponding nodes if applicable.

### Protected

- `dijkstra(...)` : implements a modified Dijkstra's algorithm that handles both edge weights and number of transfers.

- `reconstruct_path(...)` : reconstructs the shortest path found.

- `requires_transfer(...)` : determines whether a transfer is required between two adjacent nodes.

## Dependencies

Designed to be extended by classes such as [`Subway`](subway.md), which model real-world transit systems.

## Example Usage
```cpp
// NOTE: The graph class is not intended to be directly used within the simulation

Transit::Map::Graph graph{};

// id, name, train_lines, gtfs_id, latitute, longitude
graph.add_node("100", "Station 100", {TrainLine::FOUR}, {"100"}, 70.0, 70.0);
graph.add_node("101", "Station 101", {TrainLine::FOUR}, {"101"}, 50.0, 70.0);

graph.add_edge("100", "101");

Transit::Map::Path path { graph.find_path("100", "101") };
```

## Notes

### Design Decisions
- The `Graph` is implemented as a weighted, undirected graph to reflect the bidirectional nature of most transit routes.

- The `Graph` uses an adjacency list structure (`std::unordered_map<std::string, std::vector<Edge>>`) due to the sparse connectivity found in real world transit systems.

- The `Graph` class is intentionally kept lightweight and abstract, focused on only structural and pathfinding logic. Transit-specific metadata is handled in derived classes.

- A modified Dijkstra's algorithm is used for pathfinding to bias the route with both shortest path and the least amount of  transfers, i.e. the shortest and most direct route.

### Future Improvements

- The `Graph` class will be updated to implement A* pathfinding to improve pathfinding performance.