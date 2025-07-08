#pragma once

#include <string>
#include <vector>

#include "enums/train_line.h"
#include "enums/service_type.h"

namespace Transit::Map
{

    struct Node
    {
        std::string id;
        std::string name;
        std::vector<TrainLine> train_lines;
        ServiceType service_type;
        double latitude;
        double longitude;
        int degree;

        Node(const std::string &i, const std::string &n, const std::vector<TrainLine> &t, ServiceType s, double lat = 0.0, double lon = 0.0)
            : id(i), name(n), train_lines(t), service_type(s), latitude(lat), longitude(lon), degree(0) {}
    };

    struct Edge
    {
        std::string to;
        int weight;
        std::vector<TrainLine> train_lines;
        ServiceType service_type;

        Edge(const std::string &to, int w, const std::vector<TrainLine> &t, ServiceType s)
            : to(to), weight(w), train_lines(t), service_type(s) {}
    };

    class Base
    {
    private:
        std::vector<std::unique_ptr<Node>> nodes;
        std::unordered_map<std::string, Node *> node_map;
        std::unordered_map<std::string, std::vector<Edge>> adjacency_list;

    public:
        Base() = default;

        Node *add_node(const std::string &i, const std::string &n, const std::vector<TrainLine> &t, ServiceType s);
        void remove_node(const std::string &node_id);
        void remove_node(Node *u);

        void add_edge(Node *u, Node *v, int w, const std::vector<TrainLine> &t, ServiceType s);
        void remove_edge(Node *u, Node *v);

        const std::unordered_map<std::string, std::vector<Edge>> &get_adjacency_list() const;
        void print() const;
    };
}