# Centralized Traffic Control Simulator

A Centralized Traffic Control simulator that models realistic CTC operations for MTA railroads, including train movement, switch handling, signal logic, and schedule management. The simulator supports the NYC Subway, Metro North, and Long Island Railroad, simulating each system concurrently in its own designated thread. 

The simulation builds in-memory graphs of the transit systems from MTA data, instantiates all necessary resources, and generates runtime schedules and logs wrriten to the `schedule/` and `log/` directories respectively.

### Data Sources
- [Static GTFS Data](https://www.mta.info/developers)
- [MTA NYC Subway Stations](https://data.ny.gov/Transportation/MTA-Subway-Stations/39hk-dx4f/about_data)

For details about the data files, see the [data folder documentation](data/DATA.md)

For details about the data preprocessing pipeline, see the [data_pipeline folder documentation](data_pipeline/DATA_PIPELINE.md)

## Table of Contents

- [Centralized Traffic Control Simulator](#centralized-traffic-control-simulator)
- [Project Structure](#project-structure)
- [Build Instructions](#build-instructions)
  - [Requirements](#requirements)
  - [Build the Project](#build-the-project)
  - [Run the Simulation](#run-the-simulation)
  - [Run with Docker](#run-with-docker)
  - [Run Tests](#run-tests)
- [Output Paths (CMake Definitions)](#output-paths-cmake-definitions)
- [Roadmap](#roadmap)
- [License](#license)


## Project Structure

- `bin/` - compiled output binaries (set by CMake)
- `data/` - input data files (e.g MTA data)
- `data_pipeline/` - preprocessing scripts for raw data
- `include/` - header files
- `logs/` - runtime log outputs
- `src/` - implementation files
- `schedule/` - simulation generated schedules
- `tests/` - unit and integration tests

## Build Instructions

### Requirements

- CMake 3.20+
- C++20 compiler (e.g g++ 10+, clang++ 11+, MSVC 2019+)

### Build the Project
```bash
# clone repo
git clone https://github.com/ambertav/ctc-simulator.git
cd ctc-simulator

# create build directory
mkdir build && cd build

# configure and build
cmake ..
make
```

### Run the Simulation
```bash
./bin/app
```

The simulator runs with no additional configuration required. It automatically preprocesses raw data archives for the NYC Subway, Metro North, and Long Island Railroad systems by unzipping and cleaning files into the `data/` directory. Using these processed files, the simulation constructs detailed transit graphs for each rail system. All simulation schedules are written to the `schedule/` directory and all simulation events are logged to `logs/` directory, each grouped by transit system.

### Run with Docker
```bash
# build docker image
docker build -t ctc-simulator .

# run container
docker run -p 8080:8080 ctc-simulator

# see schedule
cat schedule/*.json

# see logs
cat logs/*.txt
```

### Run Tests
GoogleTest is automatically fetched via CMake. After building, run:

```bash
cd build
./tests/ctc_tests
```

Or let CMake discover and run the tests with:

```bash
cd build
ctest
```

## Output Paths (CMake Definitions)

| Variable   | Purpose                          | Default Path                |
|------------|----------------------------------|-----------------------------|
| `LOG_DIR`  | Runtime logs                     | `./logs/`                   |
| `DATA_DIR` | Input data                       | `./data/`                   |
| `SCHED_DIR`| Runtime schedule                 | `./schedule/`               |
| `bin/`     | Main executable                  | `./bin/app`                 |
| `tests/`   | Test binary                      | `./build/tests/ctc_tests`   |

## Roadmap

### Version 3.0 (Current)
- Multithreaded execution
- Concurrent simulations for MTA railroads
- Schedule construction based on transit graph
- Train movement simulation
- Switch handling and authorization workflows
- Signal logic and block safety
- Command line execution

### Version 3.5 (Upcoming)
- Randomized delays for trains to simulate real-world unpredictability
- Specialized mainte
- Aggregated logging for critical logs

## License

This project is licensed under the MIT License - see the [LICENSE](./LICENSE) file for details.