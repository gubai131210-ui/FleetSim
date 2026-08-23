#pragma once

#include "domain/map/LaneGraph.h"

#include "core/types/Path.h"

#include <optional>
#include <string>

namespace fleetsim::domain::planning {

// Lane-level route: node sequence → centerline Path (ADR-018).
class LaneRouter {
public:
    explicit LaneRouter(const map::LaneGraph& graph);

    std::optional<core::Path> route(
        const std::string& from_node_id, const std::string& to_node_id) const;

private:
    const map::LaneGraph& graph_;
};

}  // namespace fleetsim::domain::planning
