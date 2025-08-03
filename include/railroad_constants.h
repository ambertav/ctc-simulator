#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

namespace MNR
{
    struct BranchInfo
    {
        std::string_view name;
        int stop_id;
    };

    static const std::unordered_map<std::string_view, BranchInfo> branch_data{
        {"New Canaan", {"Stamford", 116}},
        {"Danbury", {"South Norwalk", 131}},
        {"Waterbury", {"Stratford", 143}}};
}

namespace LIRR
{
    static const std::unordered_map<int, std::string> city_terminal_stations{
        {237, "Penn Station"},
        {349, "Grand Central"},
        {241, "Atlantic Terminal"},
        {102, "Jamaica"},       // only a city terminal station for Oyster Bay, requires check in route building
        {90, "Hunterspoint Avenue"}};

    static const std::unordered_map<int, std::string> li_terminal_stations{
        {216, "West Hempstead"},
        {73, "Greenport"},
        {154, "Oyster Bay"},
        {65, "Far Rockaway"},
        {84, "Hempstead"},
        {141, "Montauk"},
        {27, "Babylon"},
        {113, "Long Beach"},
        {171, "Port Washington"},
        {164, "Port Jefferson"}};
}