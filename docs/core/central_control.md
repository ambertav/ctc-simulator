# CentralControl

## Overview

The `CentralControl` class coordinates the execution of the centralized traffic control simulation. Each instance operates on a specific transit system represented by the [`Graph`](graph.md) class or one of its derived classes (e.g., [`Subway`](/docs/map/subway.md), [`MetroNorth`](/docs/map/metro_north.md), [`LongIslandRailroad`](/docs/map/lirr.md)), within the `Transit::Map` namespace. The `CentralControl` constructs and owns the [`Factory`](/docs/system/factory.md) and [`Dispatch`](/docs/core/dispatch.md) classes, encapsulating the simulation's core resources and functionality. While the execution within the `CentralControl` is sequential, the class is designed to run in its own dedicated thread and executed concurrently alongside other instances.

## Responsibilities

- Manage the construction and setup of the [`Factory`](/docs/system/factory.md)
- Maintain a [`Switch`](/docs/core/switch.md) requests queue from all [`Dispatch`](/docs/core/dispatch.md) instances
- Resolve the [`Switch`](/docs/core/switch.md) queue every simulation tick and return the granted authorizations back to [`Dispatch`](/docs/core/dispatch.md)
- Run the simulation loop

## Methods

For full details, see the [header](/include/core/central_control.h) and [source](/src/core/central_control.cpp) files

### Constructor

- `CentralControl(...)` : constructs the `CentralControl` instance.

### Public

- `get_system_name()` : returns the transit system name.

- `get_dispatch(...)` : returns a raw pointer to the [`Dispatch`](/docs/core/dispatch.md) instance for a specified `TrainLine`.

- `get_granted_links(...)` : returns a vector of granted [`Switch`](/docs/core/switch.md) links for a specified [`Dispatch`](/docs/core/dispatch.md).

- `run(...)` : runs the simulation loop.

- `request_switch(...)` : enables [`Dispatch`](/docs/core/dispatch.md) to generate requests; also manages the enqueue or update by priority.
  
- `resolve_switches()` : resolves the top-priority request for each [`Switch`](/docs/core/switch.md) as applicable, and stores the granted links.

### Private

- `run_factory(...)` : constructs the [`Factory`](/docs/system/factory.md) and 
  
- `issue_dispatchers()` : constructs a [`Dispatch`](/docs/core/dispatch.md) instance for each `TrainLine`.

## Dependencies

Since the `CentralControl` owns and constructs the [`Factory`](/docs/system/factory.md) and [`Dispatch`](/docs/core/dispatch.md) classes, the `CentralControl` requires setup from both the [`Scheduler`](/docs/system/scheduler.md) and [`Registry`](/docs/system/registry.md) classes, as well as the [`Graph`](graph.md) class or one of its derived classes (e.g., [`Subway`](/docs/map/subway.md), [`MetroNorth`](/docs/map/metro_north.md), [`LongIslandRailroad`](/docs/map/lirr.md)). 

- For use in:
  - Main application

## Example Usage
```cpp
// NOTE: the CentralControl is designed to operate as its own thread
std::vector<std::future<void>> futures{};
futures.emplace_back(std::async(std::launch::async, [&]{ 

    // instantiate the CentralControl
    CentralControl central_control {system_code, system_name, graph, registry};

    for (int tick{0}; tick < last_simulation_tick; ++tick)
    {
        central_control.run(tick);
    }
}));

for (auto &f : futures)
{
    f.wait();
}
```

## Notes

### Design Decisions

- The `CentralControl` is designed to be self-contained and able to run in its own dedicated thread, mirroring the concurrent operations of distinct real-world transit systems (e.g., different railroads under MTA). Internal concurrency is deliberately avoided to reduce complexity, prevent potential race conditions, and eliminate overhead from trivial thread management.

- The `CentralControl` owns and constructs the [`Factory`](/docs/system/factory.md) and [`Dispatch`](/docs/core/dispatch.md) classes via `std::unique_ptr` for clear lifetime management and encapsulation of core resources and functionality.

- The [`Switch`](/docs/core/switch.md) request queue is implemented as an `std::unordered_map` that maps each [`Switch`](/docs/core/switch.md) pointer to its own queue of requests. Each queue itself is implemented as a `std::multimap` to maintain a sorted queue of requests by priority, including support for multiple events with the same priority. Further, the use of `std::multimap` enables efficient mid-queue update via removal and re-insertion, which ensures that previous requests can be re-issued and re-prioritized as needed.


### Future Improvements

- Introduce randomized delays and a recovery system to simulate real-world service variability.  

- Add event-driven maintenance scenarios where designated crews must be dispatched, presenting additional coordination challenges.  

- Implement aggregated logging for cross-system diagnostics and monitoring, by enabling `CentralControl` instances to bubble up critical logs into a unified system log.  
