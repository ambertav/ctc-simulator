#include <vector>
#include <iostream>
#include <filesystem>

#include "config.h"
#include "core/dispatch.h"
#include "core/platform.h"
#include "core/station.h"
#include "core/signal.h"
#include "core/track.h"
#include "core/train.h"

#include "enums/train_line.h"

#include "systems/logger.h"

void build_objects(
    std::vector<std::unique_ptr<Train>> &trains,
    std::vector<std::unique_ptr<Track>> &tracks,
    std::vector<std::unique_ptr<Platform>> &platforms,
    std::vector<std::unique_ptr<Station>> &stations,
    std::vector<std::unique_ptr<Signal>> &signals);

void build_network(std::vector<std::unique_ptr<Track>>& tracks, std::vector<std::unique_ptr<Platform>>& platforms);

int main()
{
    std::filesystem::create_directories(LOG_DIRECTORY);
    std::filesystem::create_directories(DATA_DIRECTORY);


    std::vector<std::unique_ptr<Train>> trains{};
    std::vector<std::unique_ptr<Track>> tracks{};
    std::vector<std::unique_ptr<Platform>> platforms{};
    std::vector<std::unique_ptr<Station>> stations{};
    std::vector<std::unique_ptr<Signal>> signals{};

    trains.reserve(2);
    tracks.reserve(2);
    platforms.reserve(3);
    stations.reserve(3);
    signals.reserve(5);

    build_objects(trains, tracks, platforms, stations, signals);
    build_network(tracks, platforms);

    // Logger logger{"/logs/sim.txt"};

    std::vector<Train *> train_ptrs;
    std::vector<Track *> track_ptrs;
    std::vector<Platform *> platform_ptrs;
    std::vector<Station *> station_ptrs;
    std::vector<Signal *> signal_ptrs;

    for (auto &t : trains)
        train_ptrs.push_back(t.get());
    for (auto &t : tracks)
        track_ptrs.push_back(t.get());
    for (auto &p : platforms)
        platform_ptrs.push_back(p.get());
    for (auto &s : stations)
        station_ptrs.push_back(s.get());
    for (auto &s : signals)
        signal_ptrs.push_back(s.get());
    
    Logger logger{std::string(LOG_DIRECTORY) + "/sim.txt"};
    Dispatch dispatch{station_ptrs, train_ptrs, track_ptrs, platform_ptrs, signal_ptrs, logger};

    dispatch.load_schedule(std::string(DATA_DIRECTORY) + "/schedule.csv");

    std::cout << "\n\n";

    // run loop
    for (int i = 0; i <= 20; i++)
    {
        dispatch.update(i);
    }

    return 0;
}

void build_objects(
    std::vector<std::unique_ptr<Train>> &trains,
    std::vector<std::unique_ptr<Track>> &tracks,
    std::vector<std::unique_ptr<Platform>> &platforms,
    std::vector<std::unique_ptr<Station>> &stations,
    std::vector<std::unique_ptr<Signal>> &signals)
{

    for (int i = 1; i <= 4; i++)
    {
        trains.push_back(std::make_unique<Train>(i, TrainLine::FOUR, ServiceType::EXPRESS, nullptr));
    }

    int track_id = 1;
    int signal_id = 1;
    std::vector<TrainLine> lines{TrainLine::FOUR};

    stations.push_back(std::make_unique<Station>(4000, "Yard", true, lines));
    signals.push_back(std::make_unique<Signal>(signal_id++, track_id));
    platforms.push_back(std::make_unique<Platform>(track_id++, signals[0].get(), stations[0].get(), Direction::DOWNTOWN));
    stations[0]->add_platform(platforms[0].get());

    signals.push_back(std::make_unique<Signal>(signal_id++, track_id));
    tracks.push_back(std::make_unique<Track>(track_id++, signals[1].get()));

    stations.push_back(std::make_unique<Station>(1, "42nd st", false, lines));
    signals.push_back(std::make_unique<Signal>(signal_id++, track_id));
    platforms.push_back(std::make_unique<Platform>(track_id++, signals[2].get(), stations[1].get(), Direction::DOWNTOWN));
    stations[1]->add_platform(platforms[1].get());

    signals.push_back(std::make_unique<Signal>(signal_id++, track_id));
    tracks.push_back(std::make_unique<Track>(track_id++, signals[3].get()));

    stations.push_back(std::make_unique<Station>(4001, "End Yard", true, lines));
    signals.push_back(std::make_unique<Signal>(signal_id++, track_id));
    platforms.push_back(std::make_unique<Platform>(track_id++, signals[4].get(), stations[2].get(), Direction::DOWNTOWN));
    stations[2]->add_platform(platforms[2].get());
}

void build_network(std::vector<std::unique_ptr<Track>>& tracks, std::vector<std::unique_ptr<Platform>>& platforms)
{
    // yard to track 1
    platforms[0]->set_next(tracks[0].get());
    tracks[0]->set_prev(platforms[0].get());

    // track 1 to station
    tracks[0]->set_next(platforms[1].get());
    platforms[1]->set_prev(tracks[0].get());

    // station to track 2
    platforms[1]->set_next(tracks[1].get());
    tracks[1]->set_prev(platforms[1].get());

    // track 2 to yard
    tracks[1]->set_next(platforms[2].get());
    platforms[2]->set_prev(tracks[1].get());
}