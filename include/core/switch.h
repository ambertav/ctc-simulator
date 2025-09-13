/**
 * for details on design, see:
 * docs/switch.md
 */

#pragma once

#include <vector>
#include <unordered_map>

class Signal;
class Track;

class Switch
{
private:
    std::vector<Track *> approach_tracks;
    std::vector<Track *> departure_tracks;
    std::unordered_map<Track *, Track *> links;
    const int id;

public:
    Switch(int i);

    int get_id() const;

    const std::vector<Track *> &get_approach_tracks() const;
    const std::vector<Track *> &get_departure_tracks() const;

    Track *get_link(Track *input) const;
    const std::vector<std::pair<Track *, Track *>> get_links() const;

    bool set_link(Track *input, Track *output);

    void add_approach_track(Track *tr);
    void add_departure_track(Track *tr);
    void remove_approach_track(Track *tr);
    void remove_departure_track(Track *tr);
};