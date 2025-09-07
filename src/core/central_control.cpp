#include "core/dispatch.h"
#include "core/central_control.h"

CentralControl::CentralControl(Constants::System sc, const std::string &sn, const Transit::Map::Graph &g, const Registry &r)
    : system_code(sc), system_name(sn), current_tick(0)
{
    factory = std::make_unique<Factory>();
    logger = std::make_unique<Logger>(std::string(LOG_DIRECTORY) + "/" + system_name + "/log.txt");

    run_factory(g, r);
    issue_dispatchers();
}

CentralControl::~CentralControl() = default;

std::string CentralControl::get_system_name() const
{
    return system_name;
}

std::vector<std::pair<Train *, Track *>> CentralControl::get_granted_links(Dispatch *dispatch)
{
    auto it{granted_links.find(dispatch)};
    if (it == granted_links.end())
    {
        return {};
    }
    else
    {
        return it->second;
    }
}

void CentralControl::run(int tick)
{
    current_tick = tick;

    for (const auto &dispatch : dispatchers)
    {
        dispatch->authorize(tick);
    }

    resolve_switches();

    for (const auto &dispatch : dispatchers)
    {
        dispatch->execute(tick);
    }
}

void CentralControl::request_switch(Train *train, Switch *sw, Track *from, Track *to, int priority, int tick, Dispatch *dispatch)
{
    auto existing{train_to_request.find(train)};
    if (existing != train_to_request.end())
    {
        auto &[existing_sw, request_it] = existing->second;
        if (existing_sw == sw)
        {
            SwitchRequest updated_request{request_it->second};
            updated_request.priority = priority;

            switch_requests[sw].erase(request_it);

            int age{tick - updated_request.request_tick};
            int effective_priority{priority + age};

            auto new_it{switch_requests[sw].emplace(effective_priority, updated_request)};
            existing->second = {sw, new_it};
            return;
        }
        else
        {
            switch_requests[existing_sw].erase(request_it);
        }
    }

    SwitchRequest new_request{train, sw, from, to, priority, tick, dispatch};
    auto new_it{switch_requests[sw].emplace(priority, new_request)};
    train_to_request[train] = {sw, new_it};
}

void CentralControl::resolve_switches()
{
    granted_links.clear();

    for (auto &[sw, requests] : switch_requests)
    {
        auto granted{std::ranges::find_if(requests, [](const auto &pair)
                                          {
            const auto& [priority, request] = pair;
            return request.train->is_active() && request.train->get_current_track() == request.from; })};

        if (granted != requests.end())
        {
            const auto &[priority, request] = *granted;
            if (sw->set_link(request.from, request.to))
            {
                granted_links[request.dispatch].emplace_back(request.train, request.to);
                train_to_request.erase(request.train);
                requests.erase(granted);
            }
        }
    }
}

void CentralControl::run_factory(const Transit::Map::Graph &graph, const Registry &registry)
{
    int code{static_cast<int>(system_code)};
    factory->build_network(graph, registry, code);
}

void CentralControl::issue_dispatchers()
{
    auto issue = [&](const auto &line)
    {
        std::visit([&](const auto &l)
                   {
        using T = std::decay_t<decltype(l)>;
        for (int i{0}; i < static_cast<int>(T::COUNT); ++i)
        {
            auto train_line{static_cast<T>(i)};

            const auto &trains{factory->get_trains(train_line)};
            const auto &stations{factory->get_stations(train_line)};

            auto& dispatch {dispatchers.emplace_back(std::make_unique<Dispatch>(this, train_line, stations, trains, logger.get()))};
            dispatch->load_schedule();
        } }, line);
    };

    switch (system_code)
    {
    case Constants::System::SUBWAY:
        issue(TrainLine{SUB::TrainLine{}});
        break;
    case Constants::System::METRO_NORTH:
        issue(TrainLine{MNR::TrainLine{}});
        break;
    case Constants::System::LIRR:
        issue(TrainLine{LIRR::TrainLine{}});
        break;
    }
}