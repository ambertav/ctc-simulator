#include "config.h"
#include "system_config.h"

using namespace etl;

SystemConfig create_lirr_config()
{
    return SystemConfig{
        .name = "lirr",
        .station_input_file = std::string(DATA_DIRECTORY) + "/raw/gtfslirr/stops.txt",
        .trips_input_file = std::string(DATA_DIRECTORY) + "/raw/gtfslirr/trips.txt",
        .stop_times_input_file = std::string(DATA_DIRECTORY) + "/raw/gtfslirr/stop_times.txt",
        .routes_input_file = std::string(DATA_DIRECTORY) + "/raw/gtfslirr/routes.txt",

        .station_output_file = std::string(DATA_DIRECTORY) + "/clean/lirr/stations.csv",
        .routes_output_file = std::string(DATA_DIRECTORY) + "/clean/lirr/routes.csv",

        .station_header = "stop_id,stop_code,stop_name,latitude,longitude",
        .routes_header = "route_id,headsign,ordered_stops",

        .station_columns = {"stop_id", "stop_code", "stop_name", "stop_lat", "stop_lon"},
        .trip_columns = {"route_id", "trip_id", "trip_headsign", "direction_id", "peak_offpeak"},
        .stop_time_columns = {"trip_id", "stop_id", "stop_sequence"},
        .route_columns = std::vector<std::string_view>{"route_id", "route_long_name"},

        .trip_filter = [](const auto &row)
        { return row.at("peak_offpeak") == "0" && row.at("direction_id") == "0"; },
        .transform_sequence = [](const std::vector<std::pair<int, std::string>> &stops)
        {
            std::vector<std::string> transformed{};
            transformed.reserve(stops.size());
            for (const auto& stop : stops)
            {
                transformed.push_back(stop.second);
            }
            return transformed; },

        .multiple_trips = true};
}

SystemConfig &get_lirr_config()
{
    static SystemConfig instance = create_lirr_config();
    return instance;
}