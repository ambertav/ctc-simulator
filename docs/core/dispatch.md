# Dispatch

## Overview

The `Dispatch` class provides the functionality to authorize and execute the movement of trains for a specified `TrainLine`. The `Dispatch` is owned and managed by the [`CentralControl`](/docs/core/central_control.md), which also manages [`Switch`](/docs/core/switch.md) conflicts across multiple dispatchers.

## Responsibilities

- Authorizes trains that request movement by managing signal state changes
- Calculates train priority and submits equests for [`Switch`](/docs/core/switch.md) authorizations to the [`CentralControl`](/docs/core/central_control.md)
- Orchestrates the logging of all simulation runtime events (e.g. signal changes, train arrivals, train departures, and warnings)

## Methods

For full details, see the [header](/include/core/dispatch.h) and [source](/src/core/dispatch.cpp) files

### Constructor

- `Dispatch(...)` : constructs the `Dispatch` instance.

### Public

- `get_train_line()` : returns the `TrainLine` associated with the instance.

- `get_stations()` : returns an unordered map of stations.

- `get_trains()` : returns a vector of trains.

- `get_authorizations()` : returns a vector of `Train` and `Track` pairs representing the train and the corresponding track its authorized to move into.

- `get_station_schedules()` : returns an unordered map of station id to the arrival and departure event queues
  
- `load_schedule(...)` : reads from schedule file for a specific `TrainLine` and constructs an `EventQueues` object for each station.
  
- `authorize(...)` : constructs pairs of trains and the authorized; also manages [`Switch`](/docs/core/switch.md) requests if applicable

- `execute(...)` : iterates through authorized train, track pairs and executes movement and corresponding logs.

### Private

- `process_event(...)` : finds and returns the station event from the `EventQueues`.
  
- `handle_spawns(...)` : iterates through yard ids and manages the dispatch of trains according to schedule.
  
- `spawn_train(...)` : dispatches train from yard.
  
- `despawn_train(...)` : returns train to yard after completing the route.

- `calculate_switch_priority(...)` : calculates the train's priority for the [`Switch`](/docs/core/switch.md) request queue within [`CentralControl`](/docs/core/central_control.md).
  

## Dependencies

The [`CentralControl`](/docs/core/central_control.md) owns, constructs, and manages the `Dispatch` class, as well as resolves conflicts across multiple dispatchers.

The [`Scheduler`](/docs/system/scheduler.md) class constructs the schedule to be loaded by the `Dispatch`, which constructs `EventQueues` containing arrival and departure priority queues for each station.

The [`Factory`](/docs/system/factory.md) class is responsible for creating and initializing all simulation components. The components are accessed via getter methods and passed into the `Dispatch` constructor. 

- Uses:
  - `<nlohmann/json.hpp>` to facilitate JSON formatting and parsing

- For use in:
  - [`CentralControl`](/docs/core/central_control.md)

## Example Usage
```cpp
// NOTE: the dispatch class is not intended to be used directly within the simulation
Dispatch dispatch {&central_control, train_line, stations, trains, &logger};

// load schedule for all stations within the train line
dispatch.load_schedule();

// authorize trains for the current tick
dispatch.authorize(tick);

// execute authorized train movements
dispatch.execute(tick);
```

## Notes

### Design Decisions

- The `Dispatch` holds vectors of raw pointers to objects to decouple ownership and adhere to the factory design pattern. The [`Factory`](/docs/system/factory.md) collaborates with the [`Registry`](/docs/system/registry.md) to create all necessary simulation objects. Thus, the `Dispatch` is solely responsible for managing centralized traffic control for a specified `TrainLine` within the simulation.

- The authorization and execution of movement are decoupled into separate methods within the `Dispatch` to allow for the resolution of [`Switch`](/docs/core/switch.md) requests within the [`CentralControl`](/docs/core/central_control.md) during the same simulation tick.

- The `Dispatch` class is implemented to be `TrainLine` specific with bubbling up conflicts to the [`CentralControl`](/docs/core/central_control.md) to simplify and streamline multi-line coordination with a transit system.

- Station-specific priority queues (i.e., `EventQueues`) for both arrivals and departures were chosen to manage event ordering and processing at each station independently for accurate logging and schedule adherence. Each station's `EventQueues` is accessible in O(1) with the `Station` id.

- The `EventQueues` themselves within the `Dispatch` schedule are implemented using `std::multimap` to maintain a sorted queue of events by simulation tick, including support for multiple events with the same timestamp. Further, the use of `std::multimap` enables efficient mid-queue update via removal and re-insertion.

