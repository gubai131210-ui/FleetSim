#pragma once

#include "core/types/Path.h"
#include "core/types/VehicleId.h"
#include "domain/map/OccupancyGrid.h"
#include "domain/vehicle/FleetManager.h"

#include <cstddef>
#include <string>
#include <vector>

namespace fleetsim::domain::collision {

struct CbsLiteConfig {
    int max_depth{10};
    int time_limit_ms{100};
};

struct CbsLiteResult {
    bool success{false};
    std::vector<core::Path> paths;
    int constraints_added{0};
    int nodes_expanded{0};
    std::string message;
};

/// Depth-bounded Conflict-Based Search lite (ADR-024).
/// Session 0: stub; Session 5 implements constraint tree + replan.
class CbsLiteCoordinator {
public:
    static CbsLiteResult resolve(const vehicle::FleetManager& fleet,
                                 const map::OccupancyGrid& grid,
                                 const CbsLiteConfig& config);
};

}  // namespace fleetsim::domain::collision
