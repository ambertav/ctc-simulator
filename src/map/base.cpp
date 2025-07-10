#include "map/base.h"

#include <queue>
#include <limits>

using namespace Transit::Map;

Node *Base::add_node(const std::string &i, const std::string &n, const std::vector<TrainLine> &t, const std::vector<std::string> &g, double lat, double lon)
{
    if (node_map.count(i) > 0)
    {
        throw std::invalid_argument("Node with id " + i + " already exists in transit graph");
    }
    auto new_node = std::make_unique<Node>(i, n, t, g, lat, lon);

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

void Base::add_edge(Node *u, Node *v, int w, const std::vector<TrainLine> &t)
{
    if (!node_map.count(u->id) || !node_map.count(v->id))
    {
        throw std::invalid_argument("Nodes " + u->id + " and " + v->id + " are not in the transit graph");
    }

    if (u == v)
    {
        throw std::invalid_argument("Self connections are not allowed in transit graph");
    }

    for (const auto &edge : adjacency_list[u->id])
    {
        if (edge.to == v->id)
        {
            return;
        }
    }

    adjacency_list[u->id].emplace_back(v->id, w, t);
    adjacency_list[v->id].emplace_back(u->id, w, t);

    ++u->degree;
    ++v->degree;
}

void Base::add_edge(const std::string &u, const std::string &v)
{
    if (!node_map.count(u) || !node_map.count(v))
    {
        throw std::invalid_argument("Nodes " + u + " and " + v + " are not in the transit graph");
    }

    if (u == v)
    {
        throw std::invalid_argument("Self connections are not allowed in transit graph");
    }

    Node *u_node = node_map[u];
    Node *v_node = node_map[v];

    std::vector<TrainLine> shared_lines;

    std::unordered_set<TrainLine> u_lines(u_node->train_lines.begin(), u_node->train_lines.end());

    for (const auto &line : v_node->train_lines)
    {
        if (u_lines.count(line))
        {
            shared_lines.push_back(line);
        }
    }

    add_edge(u_node, v_node, 1, shared_lines);
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

void Base::update_node(const std::string &id, const std::vector<TrainLine> &more_train_lines, const std::vector<std::string> more_gtfs_ids)
{
    auto it = node_map.find(id);
    if (it == node_map.end())
    {
        throw std::invalid_argument("Node " + id + " are not in transit graph");
    }
    Node *node = it->second;

    node->train_lines.insert(node->train_lines.end(), more_train_lines.begin(), more_train_lines.end());
    node->gtfs_ids.insert(node->gtfs_ids.end(), more_gtfs_ids.begin(), more_gtfs_ids.end());
}

const Node *Base::get_node(const std::string &id) const
{
    auto it = node_map.find(id);
    if (it != node_map.end())
    {
        return it->second;
    }

    return nullptr;
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
        std::cout << node->name << " ( id: " << id << " ): ";

        for (int i = 0; i < edges.size(); ++i)
        {
            const Edge &e = edges[i];
            const Node *neighbor = node_map.at(e.to);
            std::cout << neighbor->name << " ( id: " << e.to << " )";
            if (i != edges.size() - 1)
            {
                std::cout << ", ";
            }
        }

        std::cout << "\n";
    }
}

Path Base::find_path(const std::string &u_id, const std::string &v_id) const
{
    const Node *u = get_node(u_id);
    const Node *v = get_node(v_id);

    if (!u || !v || u == v)
    {
        std::cerr << "Nodes do not exist in transit graph\n";
        return Path{};
    }
    else
    {
        return dijkstra(u, v);
    }
}

Path Base::dijkstra(const Node *u, const Node *v) const
{
    std::unordered_map<std::string, int> dist;
    std::unordered_map<std::string, int> transfers;
    std::unordered_map<std::string, std::string> prev;
    std::unordered_set<std::string> visited;

    for (const auto &[id, _] : adjacency_list)
    {
        dist[id] = std::numeric_limits<int>::max();
    }
    dist[u->id] = 0;
    transfers[u->id] = 0;

    using PQElement = std::tuple<int /* distance */, int /* transfers */, std::string /* id */, std::vector<TrainLine>> /* train lines */;
    auto comparator = [](const PQElement &a, const PQElement &b)
    {
        if (std::get<0>(a) != std::get<0>(b)) // shorter distance
        {
            return std::get<0>(a) > std::get<0>(b);
        }
        else // then fewer transfers
        {
            return std::get<1>(a) > std::get<1>(b);
        }
    };
    std::priority_queue<PQElement, std::vector<PQElement>, decltype(comparator)> pq(comparator);

    pq.emplace(0, 0, u->id, u->train_lines);

    while (!pq.empty())
    {
        auto [current_dist, current_transfers, node_id, prev_lines] = pq.top();
        pq.pop();

        if (visited.contains(node_id))
        {
            continue;
        }
        visited.insert(node_id);

        if (node_id == v->id)
        {
            break;
        }

        for (const auto &edge : adjacency_list.at(node_id))
        {
            const std::string &neighbor_id = edge.to;
            int weight = edge.weight;
            const std::vector<TrainLine> &edge_lines = edge.train_lines;

            if (visited.contains(neighbor_id))
            {
                continue;
            }

            int new_dist = current_dist + weight;
            int new_transfers = current_transfers + (requires_transfer(prev_lines, edge_lines) ? 1 : 0);

            if (new_dist < dist[neighbor_id] || (new_dist == dist[neighbor_id] && new_transfers < transfers[neighbor_id]))
            {
                dist[neighbor_id] = new_dist;
                transfers[neighbor_id] = new_transfers;
                prev[neighbor_id] = node_id;
                pq.emplace(new_dist, new_transfers, neighbor_id, edge_lines);
            }
        }
    }

    if (dist[v->id] == std::numeric_limits<int>::max())
    {
        std::cerr << "No path exists\n";
        return Path();
    }

    return reconstruct_path(u, v, prev, transfers);
}

Path Base::reconstruct_path(const Node *u, const Node *v, const std::unordered_map<std::string, std::string> &prev, const std::unordered_map<std::string, int> &transfers) const
{
    std::vector<const Node*> path_nodes;

    std::string at = v->id;
    while (at != u->id)
    {
        auto it = prev.find(at);
    if (it == prev.end())
    {
        std::cerr << "Error reconstructing path: missing predecessor for " << at << "\n";
        return Path{};
    }
    const Node* node = get_node(at);
    if (!node) {
        std::cerr << "Error: Node with id " << at << " not found\n";
        return Path{};
    }
    path_nodes.push_back(node);
        at = it->second;
    }

    path_nodes.push_back(get_node(u->id));
    std::reverse(path_nodes.begin(), path_nodes.end());

    std::vector<int> segment_weights;
    for (int i = 0; i < path_nodes.size() - 1; ++i)
    {
        const auto &edges = adjacency_list.at(path_nodes[i]->id);
        auto it = std::find_if(edges.begin(), edges.end(), [&](const Edge &e)
                               { return e.to == path_nodes[i + 1]->id; });

        if (it != edges.end())
        {
            segment_weights.push_back(it->weight);
        }
        else
        {
            throw std::runtime_error("Missing edge between " + path_nodes[i]->id + " and " + path_nodes[i + 1]->id);
        }
    }

    int transfer_count = transfers.at(v->id);

    return Path(path_nodes, segment_weights, transfer_count);
}

bool Base::requires_transfer(const std::vector<TrainLine> &a, const std::vector<TrainLine> &b) const
{
    for (const auto &line_a : a)
    {
        for (const auto &line_b : b)
        {
            if (line_a == line_b)
                return false;
        }
    }
    return true;
}