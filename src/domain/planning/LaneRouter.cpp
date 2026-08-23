#include "LaneRouter.h"

namespace fleetsim::domain::planning {

LaneRouter::LaneRouter(const map::LaneGraph& graph)
    : graph_(graph)
{
}

std::optional<core::Path> LaneRouter::route(
    const std::string& from_node_id, const std::string& to_node_id) const
{
    const auto node_ids = graph_.shortestPath(from_node_id, to_node_id);
    if (!node_ids.has_value() || node_ids->empty()) {
        return std::nullopt;
    }

    core::Path path = graph_.centerlinePath(*node_ids);
    if (path.empty()) {
        return std::nullopt;
    }
    return path;
}

}  // namespace fleetsim::domain::planning
