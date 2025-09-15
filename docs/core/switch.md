# Switch

## Overview

The `Switch` class simulates railroad switch turnouts that control connections, or links, between converging and diverging tracks. The `Switch` class is managed by the [`CentralControl`](/docs/core/central_control.md) and influences the train movement coordinated by the [`Dispatch`](/docs/core/dispatch.md).

## Responsibilities

- Maintain an unordered map of `Track` links
- Provide methods for the [`CentralControl`](/docs/core/central_control.md) to manage `Track` links

## Methods

For full details, see the [header](/include/core/switch.h) and [source](/src/core/switch.cpp) files

### Constructor

- `Switch(...)` : constructs the `Switch` instance.

### Public

- `get_id()` : returns int id

- `get_approach_tracks()` : returns a vector of converging `Track` raw pointers.

- `get_departure_tracks()` : returns a vector of diverging `Track` raw pointers.

- `get_link(...)` : returns the raw `Track` pointer, if applicable.

- `set_link(...)` : updates the current link pair.

- `add_approach_track(...)` : adds a `Track` to the approach, or converging, tracks.

- `add_departure_track(...)` : adds a `Track` to the departure, or diverging, tracks.


## Dependencies

Designed to be owned and instantiated by the [`Factory`](/docs/system/factory.md) class and managed by the [`CentralControl`](/docs/core/central_control.md).

## Example Usage
```cpp
// instantiate the Switch instance
Switch sw {id};

// add converging and diverging tracks
sw.add_approach_track(converging);
sw.add_departure_track(diverging);

// set track links
sw.set_link(converging, diverging);

// retrieve current link mapping for a track
Track* diverging {sw.get_link(converging)};
```

## Notes

### Design Decisions

- The `Switch` class represents a physical turnout and thus the current link connection between tracks is exclusive.

- The `Switch` class maintains a pair, mapping an input `Track` to an output `Track`, allowing the [`CentralControl`](/docs/core/central_control.md) to efficiently query and update track connections at simulation runtime.

- The `Switch` class only manages runtime connectivity of tracks according to the underlying transit system network. It does not manage the ownership or generation of `Track` objects, aligning with the factory design pattern, detailed in the documentation for the  [`Factory`](/docs/system/factory.md) class. 

- Links are explicitly set through `set_link(...)` to offer the [`CentralControl`](/docs/core/central_control.md) full authority over routing decisions during runtime.