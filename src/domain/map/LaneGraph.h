#pragma once

#include "LaneTypes.h"

#include "core/types/Path.h"

#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace fleetsim::domain::map {

// Phase 8 — lane-level routing graph (ADR-018).
class LaneGraph {
public:
    bool loadFromMap(const LaneMapData& lanes);
    std::optional<std::vector<std::string>> shortestPath(
        const std::string& from_id, const std::string& to_id) const;
    std::string nearestNodeId(double x, double y) const;
    core::Path centerlinePath(const std::vector<std::string>& node_ids) const;
    std::optional<std::pair<double, double>> nodePosition(const std::string& node_id) const;

    bool empty() const { return nodes_.empty(); }
    std::size_t nodeCount() const { return nodes_.size(); }
    std::size_t edgeCount() const { return edges_.size(); }

private:
    using AdjacencyEntry = std::pair<std::size_t, double>;

    void rebuildAdjacency();
    double edgeWeight(std::size_t from_index, std::size_t to_index) const;
    std::optional<std::size_t> nodeIndex(const std::string& id) const;

    LaneMapData data_;
    std::vector<LaneNode> nodes_;
    std::vector<LaneEdge> edges_;
    std::unordered_map<std::string, std::size_t> node_index_;
    std::vector<std::vector<AdjacencyEntry>> adjacency_;
};

}  // namespace fleetsim::domain::map
