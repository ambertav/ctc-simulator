/**
 * for details on design, see:
 * docs/system/scheduler.md
 */

#pragma once

#include <string>
#include <fstream>

#include <nlohmann/json.hpp>

#include "map/graph.h"
#include "utils/utils.h"
#include "constants/constants.h"
#include "system/registry.h"

class Scheduler
{
public:
    Scheduler() = default;
    static void write_schedule(const Transit::Map::Graph &graph, Registry& registry, const std::string& outfile_subfolder, Constants::System system_code);

private:
    static void process_system(nlohmann::json &train_lines_json, const Transit::Map::Graph& graph, Registry& registry, Constants::System system_code);
    static void generate_train_schedule(nlohmann::json& schedule_json, const Transit::Map::Graph& graph, const Transit::Map::Route& route, const Info& train_info, const Info& origin_yard_info, const Info& destination_yard_info);
};