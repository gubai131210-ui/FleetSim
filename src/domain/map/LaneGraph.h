#pragma once

#include "LaneTypes.h"

#include "core/types/Path.h"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace fleetsim::domain::map {

// Phase 8 — lane-level routing graph (ADR-018).
// Session 0: API + loadFromMap storage; shortestPath/Dijkstra in Session 1.
class LaneGraph {
public:
    bool loadFromMap(const LaneMapData& lanes);
    std::optional<std::vector<std::string>> shortestPath(
        const std::string& from_id, const std::string& to_id) const;
    std::string nearestNodeId(double x, double y) const;
    core::Path centerlinePath(const std::vector<std::string>& node_ids) const;

    bool empty() const { return nodes_.empty(); }
    std::size_t nodeCount() const { return nodes_.size(); }
    std::size_t edgeCount() const { return edges_.size(); }

private:
    LaneMapData data_;
    std::vector<LaneNode> nodes_;
    std::vector<LaneEdge> edges_;
};

}  // namespace fleetsim::domain::map
