#pragma once

#include <queue>
#include <string>
#include <map>
#include <unordered_map>
#include <memory>

#include "map/graph.h"
#include "system/registry.h"

class Dispatch;
class Logger;
class Factory;
class Switch;
class Track;
class Train;

struct SwitchRequest
{
    Train *train;
    Switch *sw;
    Track *from;
    Track *to;
    int priority;
    int request_tick;
    Dispatch* dispatch;
};

class CentralControl
{
private:
    std::unique_ptr<Factory> factory;
    std::unique_ptr<Logger> logger;
    std::vector<std::unique_ptr<Dispatch>> dispatchers;
    std::string system_name;
    int system_code;
    int current_tick;

    std::unordered_map<Switch*, std::multimap<int, SwitchRequest, std::greater<int>>> switch_requests;
    std::unordered_map<Dispatch*, std::vector<std::pair<Train*, Track*>>> granted_links;
    std::unordered_map<Train *, std::multimap<int, SwitchRequest>::iterator> train_to_request;

public:
    CentralControl(int sc, const std::string &sn, const Transit::Map::Graph &g, const Registry &r);

    void run(int tick);

    void request_switch(Train *train, Switch *sw, Track *from, Track *to, int priority, int tick, Dispatch* dispatch);
    void resolve_switches();
    std::vector<std::pair<Train *, Track *>> get_granted_links(Dispatch *dispatch);

private:
    void run_factory(const Transit::Map::Graph &graph, const Registry &r);
    void setup_logger();
    void issue_dispatchers();
};