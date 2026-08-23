#include "LaneGraph.h"

namespace fleetsim::domain::map {

bool LaneGraph::loadFromMap(const LaneMapData& lanes)
{
    data_ = lanes;
    nodes_ = lanes.nodes;
    edges_ = lanes.edges;
    return true;
}

std::optional<std::vector<std::string>> LaneGraph::shortestPath(
    const std::string& /*from_id*/, const std::string& /*to_id*/) const
{
    // Session 1: Dijkstra implementation.
    return std::nullopt;
}

std::string LaneGraph::nearestNodeId(double /*x*/, double /*y*/) const
{
    // Session 1: Euclidean nearest node.
    return {};
}

core::Path LaneGraph::centerlinePath(const std::vector<std::string>& /*node_ids*/) const
{
    // Session 1–2: polyline through node coordinates.
    return {};
}

}  // namespace fleetsim::domain::map
