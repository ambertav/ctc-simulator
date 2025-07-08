#include "map/base.h"

using namespace Transit::Map;

Node *Base::add_node(const std::string &i, const std::string &n, const std::vector<TrainLine> &t, ServiceType s)
{
    if (node_map.count(i) > 0)
    {
        throw std::invalid_argument("Node with id " + i + " already exists in transit graph");
    }
    auto new_node = std::make_unique<Node>(i, n, t, s);

    Node *raw_ptr = new_node.get();
    nodes.push_back(std::move(new_node));

    node_map[i] = raw_ptr;
    adjacency_list[i] = std::vector<Edge>();

    return raw_ptr;
}

void Base::remove_node(const std::string &node_id)
{
    if (!adjacency_list.count(node_id))
    {
        throw std::invalid_argument("Node " + node_id + " is not in transit graph");
    }

    node_map.erase(node_id);

    for (auto &[id, edges] : adjacency_list)
    {
        if (id == node_id)
        {
            continue;
        }
        
        Node *neighbor = node_map[id];

        auto it = std::remove_if(edges.begin(), edges.end(), [node_id](const Edge &e)
                                 { return e.to == node_id; });

        edges.erase(it, edges.end());

        neighbor->degree = edges.size();
    }

    adjacency_list.erase(node_id);

    auto it = std::find_if(nodes.begin(), nodes.end(), [node_id](const std::unique_ptr<Node> &n)
                           { return n->id == node_id; });

    if (it != nodes.end())
    {
        nodes.erase(it);
    }
}

void Base::remove_node(Node *u)
{
    if (u == nullptr)
    {
        std::invalid_argument("Cannot remove null node");
    }

    remove_node(u->id);
}

void Base::add_edge(Node *u, Node *v, int w, const std::vector<TrainLine> &t, ServiceType s)
{
    if (!node_map.count(u->id) || !node_map.count(v->id))
    {
        throw std::invalid_argument("Nodes " + u->id + " and " + v->id + " are not in the transit graph");
    }

    if (u == v)
    {
        throw std::invalid_argument("Self connections are not allowed in transit graph");
    }

    adjacency_list[u->id].emplace_back(v->id, w, t, s);
    adjacency_list[v->id].emplace_back(u->id, w, t, s);

    ++u->degree;
    ++v->degree;
}

void Base::remove_edge(Node *u, Node *v)
{
    auto u_id = u->id;
    auto v_id = v->id;

    if (!node_map.count(u_id) || !node_map.count(v_id))
    {
        throw std::invalid_argument("Nodes " + u_id + " and " + v_id + " are not in the transit graph");
    }

    if (u == v)
    {
        throw std::invalid_argument("Self connections are not allowed in transit graph");
    }

    auto &u_edges = adjacency_list[u_id];
    auto &v_edges = adjacency_list[v_id];

    u_edges.erase(std::remove_if(u_edges.begin(), u_edges.end(), [&](const Edge &e)
                                 { return e.to == v_id; }),
                  u_edges.end());

    v_edges.erase(std::remove_if(v_edges.begin(), v_edges.end(), [&](const Edge &e)
                                 { return e.to == u_id; }),
                  v_edges.end());

    --u->degree;
    --v->degree;
}

const std::unordered_map<std::string, std::vector<Edge>> &Base::get_adjacency_list() const
{
    return adjacency_list;
}

void Base::print() const
{
    for (const auto &[id, edges] : adjacency_list)
    {
        const Node *node = node_map.at(id);
        std::cout << node->name << " (id: " << id << " ): ";

        for (int i = 0; i < edges.size(); ++i)
        {
            const Edge &e = edges[i];
            const Node *neighbor = node_map.at(e.to);
            std::cout << neighbor->name << " (id: " << e.to << " )";
            if (i != edges.size() - 1)
            {
                std::cout << ", ";
            }
        }

        std::cout << "\n";
    }
}