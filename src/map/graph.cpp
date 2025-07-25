#include "map/graph.h"

#include <queue>
#include <limits>
#include <cmath>

#include "utils.h"

using namespace Transit::Map;

Node *Graph::add_node(int i, const std::string &n, const std::vector<TrainLine> &t, const std::vector<std::string> &g, double lat, double lon)
{
    if (node_map.count(i) > 0)
    {
        throw std::invalid_argument("Node with id " + std::to_string(i) + " already exists in transit graph");
    }
    auto new_node = std::make_unique<Node>(i, n, t, g, lat, lon);

    Node *raw_ptr = new_node.get();
    nodes.push_back(std::move(new_node));

    node_map[i] = raw_ptr;
    adjacency_list[i] = std::vector<Edge>();

    return raw_ptr;
}

void Graph::remove_node(int node_id)
{
    if (!adjacency_list.count(node_id))
    {
        throw std::invalid_argument("Node " + std::to_string(node_id) + " is not in transit graph");
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

void Graph::remove_node(Node *u)
{
    if (u == nullptr)
    {
        std::invalid_argument("Cannot remove null node");
    }

    remove_node(u->id);
}

void Graph::add_edge(Node *u, Node *v, double w, const std::vector<TrainLine> &t)
{
    if (!node_map.count(u->id) || !node_map.count(v->id))
    {
        throw std::invalid_argument("Nodes " + std::to_string(u->id) + " and " + std::to_string(v->id) + " are not in the transit graph");
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

void Graph::add_edge(int u_id, int v_id)
{
    if (!node_map.count(u_id) || !node_map.count(v_id))
    {
        throw std::invalid_argument("Nodes " + std::to_string(u_id) + " and " + std::to_string(v_id) + " are not in the transit graph");
    }

    if (u_id == v_id)
    {
        throw std::invalid_argument("Self connections are not allowed in transit graph");
    }

    Node *u_node = node_map[u_id];
    Node *v_node = node_map[v_id];

    std::vector<TrainLine> shared_lines;

    std::unordered_set<TrainLine> u_lines(u_node->train_lines.begin(), u_node->train_lines.end());

    for (const auto &line : v_node->train_lines)
    {
        if (u_lines.count(line))
        {
            shared_lines.push_back(line);
        }
    }

    double weight{haversine_distance(u_node->coordinates, v_node->coordinates)};

    add_edge(u_node, v_node, weight, shared_lines);
}

void Graph::remove_edge(Node *u, Node *v)
{
    auto u_id = u->id;
    auto v_id = v->id;

    if (!node_map.count(u_id) || !node_map.count(v_id))
    {
        throw std::invalid_argument("Nodes " + std::to_string(u_id) + " and " + std::to_string(v_id) + " are not in the transit graph");
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

void Graph::update_node(int id, const std::vector<TrainLine> &more_train_lines, const std::vector<std::string> more_gtfs_ids)
{
    auto it = node_map.find(id);
    if (it == node_map.end())
    {
        throw std::invalid_argument("Node " + std::to_string(id) + " are not in transit graph");
    }
    Node *node = it->second;

    node->train_lines.insert(node->train_lines.end(), more_train_lines.begin(), more_train_lines.end());
    node->gtfs_ids.insert(node->gtfs_ids.end(), more_gtfs_ids.begin(), more_gtfs_ids.end());
}

const Node *Graph::get_node(int id) const
{
    auto it = node_map.find(id);
    if (it != node_map.end())
    {
        return it->second;
    }

    return nullptr;
}

const std::unordered_map<int, std::vector<Edge>> &Graph::get_adjacency_list() const
{
    return adjacency_list;
}

void Graph::print() const
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

std::optional<Path> Graph::find_path(int u_id, int v_id) const
{
    const Node *u = get_node(u_id);
    const Node *v = get_node(v_id);

    if (!u || !v || u == v)
    {
        std::cerr << "Nodes do not exist in transit graph\n";
        return std::nullopt;
    }
    else
    {
        return dijkstra(u, v);
    }
}

std::optional<Path> Graph::dijkstra(const Node *u, const Node *v) const
{
    std::unordered_map<int, double> dist;
    std::unordered_map<int, int> prev;
    std::unordered_set<int> visited;

    for (const auto &[id, _] : adjacency_list)
    {
        dist[id] = std::numeric_limits<double>::max();
    }
    dist[u->id] = 0.0;

    using PQElement = std::tuple<double /* distance */, int /* id */, std::vector<TrainLine> /* train lines */>;
    auto comparator = [](const PQElement &a, const PQElement &b)
    {
        return std::get<0>(a) > std::get<0>(b);
    };

    std::priority_queue<PQElement, std::vector<PQElement>, decltype(comparator)> pq(comparator);
    pq.emplace(0.0, u->id, u->train_lines);

    while (!pq.empty())
    {
        auto [current_dist, node_id, prev_lines] = pq.top();
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
            const int &neighbor_id{edge.to};
            double weight{edge.weight};
            const std::vector<TrainLine> &edge_lines{edge.train_lines};

            if (visited.contains(neighbor_id))
            {
                continue;
            }

            double transfer_penalty{requires_transfer(prev_lines, edge_lines) ? Utils::TRANSFER_EPSILON : 0.0};
            double new_dist = current_dist + weight + transfer_penalty;

            if (new_dist < dist[neighbor_id])
            {
                dist[neighbor_id] = new_dist;
                prev[neighbor_id] = node_id;
                pq.emplace(new_dist, neighbor_id, edge_lines);
            }
        }
    }

    if (dist[v->id] == std::numeric_limits<double>::max())
    {
        std::cerr << "No path exists\n";
        return std::nullopt;
    }

    return reconstruct_path(u, v, prev);
}

std::optional<Path> Graph::reconstruct_path(const Node *u, const Node *v, const std::unordered_map<int, int> &prev) const
{
    std::vector<const Node *> path_nodes;

    auto at = v->id;
    while (at != u->id)
    {
        auto it = prev.find(at);
        if (it == prev.end())
        {
            std::cerr << "Error reconstructing path: missing predecessor for " << at << "\n";
            return std::nullopt;
        }

        const Node *node = get_node(at);
        if (!node)
        {
            std::cerr << "Error: Node with id " << at << " not found\n";
            return std::nullopt;
        }

        path_nodes.push_back(node);
        at = it->second;
    }

    path_nodes.push_back(get_node(u->id));
    std::reverse(path_nodes.begin(), path_nodes.end());

    std::vector<double> segment_weights;
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
            throw std::runtime_error("Missing edge between " + std::to_string(path_nodes[i]->id) + " and " + std::to_string(path_nodes[i + 1]->id));
        }
    }

    return Path(path_nodes, segment_weights);
}

double Graph::haversine_distance(const Coordinate &from, const Coordinate &to)
{
    using namespace Utils;

    double from_lat_rad{from.latitude * DEG_TO_RAD};
    double from_lon_rad{from.longitude * DEG_TO_RAD};

    double to_lat_rad{to.latitude * DEG_TO_RAD};
    double to_lon_rad{to.longitude * DEG_TO_RAD};

    double diff_lat{to_lat_rad - from_lat_rad};
    double diff_lon{to_lon_rad - from_lon_rad};

    double a{sin(diff_lat / 2.0) * sin(diff_lat / 2.0) +
             cos(from_lat_rad) * cos(to_lat_rad) *
                 sin(diff_lon / 2.0) * sin(diff_lon / 2.0)};

    double c{2.0 * atan2(sqrt(a), sqrt(1.0 - a))};

    return EARTH_RADIUS_KM * c;
}

bool Graph::requires_transfer(const std::vector<TrainLine> &a, const std::vector<TrainLine> &b) const
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