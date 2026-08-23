#include "LaneGraph.h"

#include "core/types/Waypoint.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>

namespace fleetsim::domain::map {

namespace {

double euclideanDistance(double x0, double y0, double x1, double y1)
{
    const double dx = x1 - x0;
    const double dy = y1 - y0;
    return std::sqrt(dx * dx + dy * dy);
}

}  // namespace

bool LaneGraph::loadFromMap(const LaneMapData& lanes)
{
    data_ = lanes;
    nodes_ = lanes.nodes;
    edges_ = lanes.edges;
    rebuildAdjacency();
    return true;
}

void LaneGraph::rebuildAdjacency()
{
    node_index_.clear();
    for (std::size_t i = 0; i < nodes_.size(); ++i) {
        node_index_[nodes_[i].id] = i;
    }

    adjacency_.assign(nodes_.size(), {});

    const auto addDirectedEdge = [this](const std::string& from_id, const std::string& to_id) {
        const auto from_it = node_index_.find(from_id);
        const auto to_it = node_index_.find(to_id);
        if (from_it == node_index_.end() || to_it == node_index_.end()) {
            return;
        }
        const double weight = edgeWeight(from_it->second, to_it->second);
        adjacency_[from_it->second].emplace_back(to_it->second, weight);
    };

    for (const LaneEdge& edge : edges_) {
        addDirectedEdge(edge.from, edge.to);
        if (edge.bidirectional) {
            addDirectedEdge(edge.to, edge.from);
        }
    }
}

double LaneGraph::edgeWeight(std::size_t from_index, std::size_t to_index) const
{
    const LaneNode& from = nodes_[from_index];
    const LaneNode& to = nodes_[to_index];
    return euclideanDistance(from.x, from.y, to.x, to.y);
}

std::optional<std::size_t> LaneGraph::nodeIndex(const std::string& id) const
{
    const auto it = node_index_.find(id);
    if (it == node_index_.end()) {
        return std::nullopt;
    }
    return it->second;
}

std::optional<std::vector<std::string>> LaneGraph::shortestPath(
    const std::string& from_id, const std::string& to_id) const
{
    const auto from_index = nodeIndex(from_id);
    const auto to_index = nodeIndex(to_id);
    if (!from_index.has_value() || !to_index.has_value()) {
        return std::nullopt;
    }

    if (*from_index == *to_index) {
        return std::vector<std::string>{from_id};
    }

    const std::size_t node_count = nodes_.size();
    std::vector<double> best_cost(node_count, std::numeric_limits<double>::infinity());
    std::vector<std::optional<std::size_t>> predecessor(node_count, std::nullopt);

    using QueueEntry = std::pair<double, std::size_t>;
    std::priority_queue<QueueEntry, std::vector<QueueEntry>, std::greater<QueueEntry>> queue;

    best_cost[*from_index] = 0.0;
    queue.emplace(0.0, *from_index);

    while (!queue.empty()) {
        const QueueEntry current = queue.top();
        queue.pop();

        const double cost = current.first;
        const std::size_t node = current.second;
        if (cost > best_cost[node]) {
            continue;
        }
        if (node == *to_index) {
            break;
        }

        for (const AdjacencyEntry& edge : adjacency_[node]) {
            const std::size_t neighbor = edge.first;
            const double next_cost = cost + edge.second;
            if (next_cost < best_cost[neighbor]) {
                best_cost[neighbor] = next_cost;
                predecessor[neighbor] = node;
                queue.emplace(next_cost, neighbor);
            }
        }
    }

    if (!std::isfinite(best_cost[*to_index])) {
        return std::nullopt;
    }

    std::vector<std::string> path;
    for (std::optional<std::size_t> cursor = to_index; cursor.has_value();) {
        path.push_back(nodes_[*cursor].id);
        cursor = predecessor[*cursor];
    }
    std::reverse(path.begin(), path.end());
    return path;
}

std::string LaneGraph::nearestNodeId(double x, double y) const
{
    if (nodes_.empty()) {
        return {};
    }

    std::string closest_id = nodes_.front().id;
    double best_distance = euclideanDistance(x, y, nodes_.front().x, nodes_.front().y);

    for (std::size_t i = 1; i < nodes_.size(); ++i) {
        const double distance = euclideanDistance(x, y, nodes_[i].x, nodes_[i].y);
        if (distance < best_distance) {
            best_distance = distance;
            closest_id = nodes_[i].id;
        }
    }
    return closest_id;
}

core::Path LaneGraph::centerlinePath(const std::vector<std::string>& node_ids) const
{
    std::vector<core::Waypoint> waypoints;
    waypoints.reserve(node_ids.size());

    for (const std::string& node_id : node_ids) {
        const auto index = nodeIndex(node_id);
        if (!index.has_value()) {
            return {};
        }
        const LaneNode& node = nodes_[*index];
        waypoints.push_back(core::Waypoint{node.x, node.y});
    }

    return core::Path(std::move(waypoints));
}

std::optional<std::pair<double, double>> LaneGraph::nodePosition(const std::string& node_id) const
{
    const auto index = nodeIndex(node_id);
    if (!index.has_value()) {
        return std::nullopt;
    }
    const LaneNode& node = nodes_[*index];
    return std::make_pair(node.x, node.y);
}

}  // namespace fleetsim::domain::map
