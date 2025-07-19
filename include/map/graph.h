#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <numeric>
#include <memory>
#include <optional>

#include "enums/transit_types.h"
#include "constants.h"
#include "enums/service_type.h"

namespace Transit::Map
{

    struct Coordinate
    {
        double latitude = 0.0;
        double longitude = 0.0;

        Coordinate(double lat, double lon) : latitude(lat), longitude(lon) {}
    };

    struct Node
    {
        std::string id;
        std::string name;
        std::vector<TrainLine> train_lines;
        std::vector<std::string> gtfs_ids;
        Coordinate coordinates;
        int degree;

        Node(const std::string &i, const std::string &n, const std::vector<TrainLine> &t, const std::vector<std::string>& g, const Coordinate& c)
            : id(i), name(n), train_lines(t), gtfs_ids(g), coordinates(c), degree(0) {}

        Node(const std::string &i, const std::string &n, const std::vector<TrainLine> &t, const std::vector<std::string>& g, double lat, double lon)
            : Node(i, n, t, g, Coordinate{lat, lon}) {}
    };

    struct Edge
    {
        std::string to;
        double weight;
        std::vector<TrainLine> train_lines;

        Edge(const std::string &to, double w, const std::vector<TrainLine> &t)
            : to(to), weight(w), train_lines(t) {}
    };

    struct Path
    {
        std::vector<const Node*> nodes;
        std::vector<double> segment_weights;
        double total_weight = 0.0;

        Path() = default;

        Path(const std::vector<const Node*> &n, const std::vector<double> &sw) : nodes(n), segment_weights(sw)
        {
            total_weight = std::accumulate(sw.begin(), sw.end(), 0.0);
        }
    };

    class Graph
    {
    private:
        std::vector<std::unique_ptr<Node>> nodes;
        std::unordered_map<std::string, Node *> node_map;
        std::unordered_map<std::string, std::vector<Edge>> adjacency_list;

    public:
        Graph() = default;

        Node *add_node(const std::string &i, const std::string &n, const std::vector<TrainLine> &t, const std::vector<std::string>& g, double lat = 0.0, double lon = 0.0);
        void remove_node(const std::string &node_id);
        void remove_node(Node *u);

        void add_edge(Node *u, Node *v, double w, const std::vector<TrainLine> &t);
        void add_edge(const std::string& u, const std::string& v);
        void remove_edge(Node *u, Node *v);

        void update_node(const std::string &id, const std::vector<TrainLine> &more_train_lines, const std::vector<std::string> more_gtfs_ids);

        const Node *get_node(const std::string &id) const;
        const std::unordered_map<std::string, std::vector<Edge>> &get_adjacency_list() const;
        void print() const;

        std::optional<Path> find_path(const std::string &u_id, const std::string &v_id) const;

    protected:
        std::optional<Path> dijkstra(const Node *u, const Node *v) const;
        std::optional<Path> reconstruct_path(const Node *u, const Node *v, const std::unordered_map<std::string, std::string> &prev) const;

        double haversine_distance(const Coordinate& from, const Coordinate& to);
        bool requires_transfer(const std::vector<TrainLine> &a, const std::vector<TrainLine> &b) const;
    };
}