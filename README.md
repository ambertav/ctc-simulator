# Centralized Traffic Control Simulator

A Centralized Traffic Control simulator that models realistic train movement, signal logic, and schedule management using MTA data. The current version allows for the selection of any two NYC Subway stations, from which the simulator creates a schedule, instantiates all necessary objects, and manages the simulation through the dispatcher. The schedule and runtime logs are available in `data/schedule.csv` and `logs/sim.txt` respectively.

### Data Sources
- [MTA NYC Subway Stations](https://data.ny.gov/Transportation/MTA-Subway-Stations/39hk-dx4f/about_data)
- [Static GTFS Data](https://www.mta.info/developers)

For details about the data files, see the [data folder documentation](data/DATA.md)

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

- `src/` - implementation files
- `include/` - header files
- `tests/` - unit and integration tests
- `bin/` - compiled output binaries (set by CMake)
- `data/` - input data files (e.g schedules, map config)
- `logs/` - runtime log outputs

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

# configure with cmake
cmake ..

# build executable and tests
make
```

### Run the Simulation
```bash
./bin/app
```

The simulator runs with no additional configuration required. It automatically reads subway map data from files provided by the MTA located in the `data/` directory to construct the NYC Subway System for simulation. All simulation details are logged to `logs/sim.txt`.

### Run with Docker
```bash
# build docker image
docker build -t ctc-simulator .

# run container
docker run -p 8080:8080 ctc-simulator

# see schedule
cat data/schedule.csv

# see logs
cat logs/sim.txt
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

| Variable   | Purpose                         | Default Path               |
|------------|----------------------------------|-----------------------------|
| `LOG_DIR`  | Runtime logs                    | `./logs/`                   |
| `DATA_DIR` | Input data (e.g., schedule)     | `./data/`                   |
| `bin/`     | Main app output                 | `./bin/app`                 |
| `tests/`   | Test binary output              | `./build/tests/ctc_tests`   |

## Roadmap

### Version 2.0 (Current)
- Pathfinding between any two NYC Subway stations
- Schedule construction based on path
- Train movement simulation
- Signal logic and block safety
- Command line execution

### Version 2.5 (Upcoming)
- Simulation compatibility with Metro-North
- Simulation compatibility with LIRR

### Version 3.0 (Upcoming)
- Randomized delays for trains to simulate real-world unpredictability

## License

This project is licensed under the MIT License - see the [LICENSE](./LICENSE) file for details.