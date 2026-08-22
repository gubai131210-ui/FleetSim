#pragma once

#include "core/types/Path.h"
#include "core/types/VehicleId.h"
#include "domain/map/OccupancyGrid.h"
#include "domain/vehicle/FleetManager.h"

#include <cstddef>
#include <vector>

namespace fleetsim::domain::collision {

/// Priority-based multi-agent path coordination (ADR-013).
/// Higher-priority agents plan/reserve first; lower-priority planning grids
/// treat higher-priority committed paths as static occupancy. Does NOT replace
/// TimeWindowCollisionAvoidance — it only decides corridor ownership order.
class PriorityPathCoordinator {
public:
    /// Higher task_priority first; ties broken by vehicle id ascending.
    static std::vector<std::size_t> orderedAgentIndices(const vehicle::FleetManager& fleet);

    /// True if (priority_a, id_a) should plan before (priority_b, id_b).
    static bool hasPriorityOver(int priority_a,
                                const core::VehicleId& id_a,
                                int priority_b,
                                const core::VehicleId& id_b);

    /// Mark path cells occupied on grid (optional Chebyshev inflate).
    static void paintPathOccupied(map::OccupancyGrid& grid,
                                  const core::Path& path,
                                  int inflate_cells = 1);
};

}  // namespace fleetsim::domain::collision
