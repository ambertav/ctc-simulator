#include <format>

#include "utils/utils.h"
#include "core/dispatch.h"
#include "core/agency_control.h"

AgencyControl::AgencyControl(Constants::System sc, const std::string &sn, const Transit::Map::Graph &g, const Registry &r, CentralLogger& cl)
    : system_code(sc), system_name(sn), current_tick(0)
{
    factory = std::make_unique<Factory>();
    logger = std::make_unique<Logger>(std::string(LOG_DIRECTORY) + "/" + system_name + "/log.txt", sc, cl);

    run_factory(g, r);
    issue_dispatchers();
}

AgencyControl::~AgencyControl() = default;

std::string AgencyControl::get_system_name() const
{
    return system_name;
}

Dispatch *AgencyControl::get_dispatch(TrainLine train_line) const
{
    auto it{std::ranges::find_if(dispatchers, [train_line](auto &dispatch)
                                 { return trainlines_equal(dispatch->get_train_line(), train_line); })};

    if (it != dispatchers.end())
    {
        return it->get();
    }
    else
    {
        return nullptr;
    }
}

std::vector<std::pair<Train *, Track *>> AgencyControl::get_granted_links(Dispatch *dispatch)
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

void AgencyControl::run(int tick)
{
    current_tick = tick;

    std::erase_if(failed_switches, [&](Switch *sw)
                  {
    sw->update_repair();
    if (sw->is_functional())
    {
        logger->critical(std::format("Switch {} restored at tick {}", sw->get_id(), current_tick));
        return true;
    }
    else
    {
        return false;
    }});

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

void AgencyControl::request_switch(Train *train, Switch *sw, Track *from, Track *to, int priority, int tick, Dispatch *dispatch)
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
            if (switch_requests[existing_sw].empty())
            {
                switch_requests.erase(existing_sw);
            }
            train_to_request.erase(existing);
        }
    }

    SwitchRequest new_request{train, sw, from, to, priority, tick, dispatch};
    auto new_it{switch_requests[sw].emplace(priority, new_request)};
    train_to_request[train] = {sw, new_it};
}

void AgencyControl::resolve_switches()
{
    granted_links.clear();

    for (auto &[sw, requests] : switch_requests)
    {
        if (sw->is_functional())
        {
            bool failed{Utils::coin_flip(Constants::SWITCH_FAILURE_PROBABILITY)};
            if (failed)
            {
                int time_to_repair{std::max<int>(1, Utils::random_in_range(Constants::MAX_DELAY))};
                sw->set_failure(time_to_repair);
                failed_switches.insert(sw);

                logger->critical(std::format("Switch {} failure at tick {}", sw->get_id(), current_tick));
                continue;
            }

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
}

void AgencyControl::run_factory(const Transit::Map::Graph &graph, const Registry &registry)
{
    factory->build_network(graph, registry, system_code);
}

void AgencyControl::issue_dispatchers()
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