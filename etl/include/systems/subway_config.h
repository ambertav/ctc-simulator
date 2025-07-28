#include "config.h"
#include "system_config.h"

using namespace etl;

SystemConfig create_subway_config()
{
    return SystemConfig
    {
        .name = "subway",
        .station_input_file = std::string(DATA_DIRECTORY) + "/raw/mta_subway_stations.csv",
        .trips_input_file = std::string(DATA_DIRECTORY) + "/raw/gtfs_subway/trips.txt",
        .stop_times_input_file = std::string(DATA_DIRECTORY) + "/raw/gtfs_subway/stop_times.txt",
        .routes_input_file = std::nullopt,

        .station_output_file = std::string(DATA_DIRECTORY) + "/clean/subway/stations.csv",
        .routes_output_file = std::string(DATA_DIRECTORY) + "/clean/subway/routes.csv",

        .station_header = "complex_id,gtfs_id,stop_name,train_lines,latitude,longitude",
        .routes_header = "route_id,headsign,ordered_stops",

        .station_columns = {"Complex ID", "GTFS Stop ID", "Stop Name", "Daytime Routes", "GTFS Latitude", "GTFS Longitude"},
        .trip_columns = {"route_id", "trip_id", "service_id", "trip_headsign"},
        .stop_time_columns = {"trip_id", "stop_id", "stop_sequence"},
        .route_columns = std::nullopt,

        .trip_filter = [](const auto &row)
        {
            return row.at("service_id") == "Saturday" && (!row.at("route_id").empty() && row.at("route_id").back() != 'X');
        },
        .transform_sequence = [](const std::vector<std::pair<int, std::string>>& stops)
        {
            std::vector<std::string> transformed{};
            transformed.reserve(stops.size());
            for (const auto& stop : stops)
            {
                std::string id {stop.second};
                if (!id.empty())
                {
                    id = id.substr(0, id.size() - 1);
                }
                transformed.push_back(id);
            }
            return transformed;
        },
        
        .multiple_trips = false,
    };
}

SystemConfig &get_subway_config()
{
    static SystemConfig instance = create_subway_config();
    return instance;
}