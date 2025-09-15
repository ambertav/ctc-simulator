#include <string>
#include <fstream>

#include "config.h"
#include "constants/constants.h"
#include "enum/transit_types.h"
#include "system/scheduler.h"

void Scheduler::write_schedule(const Transit::Map::Graph &graph, const Registry &registry, const std::string &outfile_subfolder, int system_code)
{
    using json = nlohmann::json;

    json output{};
    output["train_lines"] = json::object();

    std::string file_path{std::string(SCHED_DIRECTORY) + "/" + outfile_subfolder + "/schedule.json"};

    try
    {
        process_system(output["train_lines"], graph, registry, system_code);

        std::ofstream outfile(file_path, std::ios::out | std::ios::trunc);
        if (!outfile.is_open())
        {
            throw std::runtime_error("Failed to open output file: " + file_path);
        }

        outfile << output.dump(2);
        outfile.flush();
        outfile.close();
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error("Failed to create schedule for system: " + std::to_string(system_code) + ", " + outfile_subfolder + std::string(e.what()));
    }
}

void Scheduler::process_system(nlohmann::json &train_lines_json, const Transit::Map::Graph &graph, const Registry &registry, int system_code)
{
    using json = nlohmann::json;

    const auto &train_registry{registry.get_train_registry(system_code)};
    const auto &yard_registry{registry.get_yard_registry(system_code)};

    const auto &routes_map{graph.get_routes()};

    // TO-DO: prepares trainline to yard pair mapping, consider this being default in registry
    std::unordered_map<TrainLine, std::pair<int, int>> yard_map{};
    for (const auto &yard_pair : yard_registry)
    {
        Info yard_info{registry.decode(yard_pair.first)};
        yard_map[yard_info.train_line] = yard_pair;
    }

    for (int i{0}; i < train_registry.size(); ++i)
    {
        int train_id{train_registry[i]};
        Info train_info{registry.decode(train_id)};

        auto yard_map_it{yard_map.find(train_info.train_line)};
        if (yard_map_it == yard_map.end())
        {
            continue;
        }

        Info first_yard_info{registry.decode(yard_map_it->second.first)};   // uptown and manhattan
        Info second_yard_info{registry.decode(yard_map_it->second.second)}; // downtown and away from manhattan

        Info origin_yard_info{};
        Info destination_yard_info{};

        if (directions_equal(train_info.direction, second_yard_info.direction))
        {
            origin_yard_info = first_yard_info;
            destination_yard_info = second_yard_info;
        }
        else
        {
            origin_yard_info = second_yard_info;
            destination_yard_info = first_yard_info;
        }

        auto routes_it{routes_map.find(train_info.train_line)};
        if (routes_it == routes_map.end())
        {
            continue;
        }

        std::vector<const Transit::Map::Route *> matching_routes{};
        for (const auto &route : routes_it->second)
        {
            if (directions_equal(route.direction, train_info.direction))
            {
                matching_routes.push_back(&route);
            }
        }

        if (matching_routes.empty())
        {
            continue;
        }

        size_t index{Utils::random_index(matching_routes.size())};
        const Transit::Map::Route *matching_route{matching_routes[index]};

        json trains_json{};
        trains_json["train_id"] = train_id;
        trains_json["direction"] = direction_to_string(train_info.direction);
        trains_json["headsign"] = matching_route->headsign;
        trains_json["schedule"] = json::array();

        generate_train_schedule(trains_json["schedule"], graph, *matching_route, train_info, origin_yard_info, destination_yard_info);

        std::string line_name{trainline_to_string(train_info.train_line)};
        train_lines_json[line_name]["trains"].push_back(trains_json);
    }
}

void Scheduler::generate_train_schedule(nlohmann::json &schedule_json, const Transit::Map::Graph &graph, const Transit::Map::Route &route, const Info &train_info, const Info &origin_yard_info, const Info &destination_yard_info)
{
    using json = nlohmann::json;

    int current_tick{train_info.instance * Constants::YARD_DEPARTURE_GAP};

    json origin{};
    origin["station_id"] = origin_yard_info.id;
    origin["station_name"] = Utils::generate_yard_name(origin_yard_info);
    origin["arrival_tick"] = -1;
    origin["departure_tick"] = current_tick;
    schedule_json.push_back(origin);

    current_tick += Constants::DEFAULT_TRAVEL_TIME;

    for (int i{0}; i < route.sequence.size(); ++i)
    {
        const Transit::Map::Node *node{graph.get_node(route.sequence[i])};
        if (!node)
        {
            continue;
        }

        json stop;
        stop["station_id"] = node->id;
        stop["station_name"] = node->name;
        stop["arrival_tick"] = current_tick;

        current_tick += Constants::STATION_DWELL_TIME;
        stop["departure_tick"] = current_tick;
        schedule_json.push_back(stop);

        if (i < route.distances.size())
        {
            current_tick += route.distances[i];
        }
        else
        {
            current_tick += Constants::DEFAULT_TRAVEL_TIME;
        }
    }

    json end{};
    end["station_id"] = destination_yard_info.id;
    end["station_name"] = Utils::generate_yard_name(destination_yard_info);
    end["arrival_tick"] = current_tick;
    end["departure_tick"] = -1;
    schedule_json.push_back(end);
}