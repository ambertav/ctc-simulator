# Registry

## Overview

The `Registry` class generates and stores encoded integer ids for trains and yards per transit system, primarily for retrieval by the [`Factory`](/docs/system/factory.md) and the [`Scheduler`](/docs/system/scheduler.md). The `Registry` is the single source of truth, ensuring that scheduling details in the [`Scheduler`](/docs/system/scheduler.md) are consistent with actual resources created in the [`Factory`](/docs/system/factory.md). The class also provides and a `decode(...)` method to extract the `Info`.

## Responsibilities

- Generates train and yard id registries by encoding metadata for each transit system.

## Methods

For full details, see the [header](/include/systems/registry.h) and [source](/src/systems/registry.cpp) files

### Constructor

- `Registry()` : private constructor implementing the singleton pattern.

### Public

- `get_instance()` : returns the singleton instance.

- `get_train_registry(...)` : returns a vector of encoded train ids for a specified rail system.

- `get_yard_registry(...)` : returns a vector of encoded yard id pairs for a specified rail system; each pair is ordered by `Direction`.

- `encode(...)` : encodes system code, `TrainLine`, `Direction`, and instance into a 32-bit integer id.

- `decode(...)` : decodes a 32-bit integer id into its system code, `TrainLine`, `Direction`, and instance components.
  

### Private

- `build_registry()` : invoked by the constructor to populate the registries.

- `generate_trains(...)` : generates, encodes, and stores train ids for each `TrainLine` and `Direction`.

- `generate_yards(...)` : generates, encodes, and stores yard id pairs for each `TrainLine`, ordered by `Direction`.

## Dependencies

The `Registry` uses system codes and corresponding `Direction` order defined in the `Constants` namespace to categorize and encode resources.

- For use in:
  - Main application

## Example Usage
```cpp
// get singleton instance of Registry
Registry &registry {Registry::get_instance()};

// retrieve train and yard registries per rail system
const auto &train_registry {registry.get_train_registry(system_code)};
const auto &yard_registry {registry.get_yard_registry(system_code)};

// decode Info using encoded ids
Info train_info {registry.decode(train_id)};
Info yard_info {registry.decode(yard_pair.first)};
```

## Notes

### Design Decisions

- The `Registry` class is implemented as a singleton to ensure consistency across the simulation and avoid redundant memory usage and initialization overhead.

- The `Registry` uses 32-bit integer encoding for efficient storage and lookup metadata pertinent to the simulation runtime, such as system code, `TrainLine`, `Direction`, and instance, with the instance attribute ensuring that each id is unique.

- The `Constants` namespace provides the source of truth for the directional order of yard pairs for the `Registry` class and subsequent classes that utilize it.
