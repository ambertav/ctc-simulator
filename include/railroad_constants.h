#pragma once

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