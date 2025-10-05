#pragma once

#include <queue>
#include <string>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <memory>

#include "constants/constants.h"
#include "map/graph.h"
#include "core/train.h"
#include "core/switch.h"
#include "system/logger.h"
#include "system/factory.h"
#include "system/registry.h"

class Dispatch;

struct SwitchRequest
{
    Train *train;
    Switch *sw;
    Track *from;
    Track *to;
    int priority;
    int request_tick;
    Dispatch *dispatch;
};

class AgencyControl
{
private:
    std::vector<std::unique_ptr<Dispatch>> dispatchers;
    std::string system_name;
    std::unique_ptr<Factory> factory;
    std::unique_ptr<Logger> logger;
    Constants::System system_code;
    int current_tick;

    std::unordered_set<Switch *> failed_switches;
    std::unordered_map<Switch *, std::multimap<int /* priority */, SwitchRequest, std::greater<int>>> switch_requests;
    std::unordered_map<Dispatch *, std::vector<std::pair<Train *, Track *>>> granted_links;
    std::unordered_map<Train *, std::pair<Switch *, std::multimap<int /* priority */, SwitchRequest>::iterator>> train_to_request;

public:
    AgencyControl(Constants::System sc, const std::string &sn, const Transit::Map::Graph &g, const Registry &r);
    ~AgencyControl();

    std::string get_system_name() const;
    Dispatch *get_dispatch(TrainLine train_line) const;
    std::vector<std::pair<Train *, Track *>> get_granted_links(Dispatch *dispatch);

    void run(int tick);

    void request_switch(Train *train, Switch *sw, Track *from, Track *to, int priority, int tick, Dispatch *dispatch);
    void resolve_switches();

private:
    void run_factory(const Transit::Map::Graph &graph, const Registry &r);
    void issue_dispatchers();
};