# Data Directory Overview

The `data/` folder contains all input files used by the Centralized Traffic Control Simulator to model the transit systems. This includes transit schedules, station metadata, and routing information.

## Preprocessing

The simulator uses a multi-stage preprocessing pipeline to reduce overhead. This process involves unzipping raw data archives and writing cleaned, standardized files to the `data/` directory to speed up file I/O during the simulation.

For details on the data preprocessing pipeline, see the [data_pipeline folder documentation](../data_pipeline/DATA_PIPELINE.md)

## Files and Directories

### zips/
- gtfs_subway.zip
  - Description: contains original GTFS files for NYC Subway, downloaded from MTA.
  - Source: [Static GTFS Data](https://www.mta.info/developers)

- gtfslirr.zip
  - Description: contains original GTFS files for the Long Island Railroad, downloaded from MTA.
  - Source: [Static GTFS Data](https://www.mta.info/developers)
  
- gtfsmnr.zip
  - Description: contains original GTFS files for the Metro North Railroad, downloaded from MTA.
  - Source: [Static GTFS Data](https://www.mta.info/developers)
  
- mta_subway_stations.csv.zip
  - Description: contains subway station data aggregated by `complex_id`.
  - Notes: helps prevent duplicate nodes while still retaining reference to `gtfs_id`.
  - Source: [MTA NYC Subway Stations](https://data.ny.gov/Transportation/MTA-Subway-Stations/39hk-dx4f/about_data)

### raw/

Contains decompressed folders and files after running the `zip_extractor/`.

- gtfs_subway/

- gtfslirr/
  
- gtfsmnr/

- mta_subway_stations.csv

### clean/

Contains processed, standardized outputs created after running the `etl/`.

- lirr/
  - routes.csv
  - stations.csv
  
- mnr/
  - routes.csv
  - stations.csv

- subway/
  - routes.csv
  - stations.csv

## Usage

The data files are loaded at runtime by various modules:

- [`Subway`](/docs/subway.md) reads station and connection files to build the transit graph.
  
- [`MetroNorth`](/docs/metro_north.md) reads station and connection files to build the transit graph.
  
- [`LongIslandRailroad`](/docs/lirr.md) reads station and connection files to build the transit graph.