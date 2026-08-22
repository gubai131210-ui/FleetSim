#include "domain/collision/PriorityPathCoordinator.h"

#include <algorithm>
#include <cmath>

namespace fleetsim::domain::collision {

bool PriorityPathCoordinator::hasPriorityOver(int priority_a,
                                              const core::VehicleId& id_a,
                                              int priority_b,
                                              const core::VehicleId& id_b)
{
    if (priority_a != priority_b) {
        return priority_a > priority_b;
    }
    return id_a < id_b;
}

std::vector<std::size_t> PriorityPathCoordinator::orderedAgentIndices(
    const vehicle::FleetManager& fleet)
{
    std::vector<std::size_t> indices;
    indices.reserve(fleet.count());
    for (std::size_t i = 0; i < fleet.count(); ++i) {
        indices.push_back(i);
    }

    std::sort(indices.begin(), indices.end(), [&](std::size_t lhs, std::size_t rhs) {
        const vehicle::VehicleAgent& a = fleet.agent(lhs);
        const vehicle::VehicleAgent& b = fleet.agent(rhs);
        const core::VehicleId& id_a = a.vehicle != nullptr ? a.vehicle->id() : core::VehicleId{};
        const core::VehicleId& id_b = b.vehicle != nullptr ? b.vehicle->id() : core::VehicleId{};
        return hasPriorityOver(a.task_priority, id_a, b.task_priority, id_b);
    });
    return indices;
}

void PriorityPathCoordinator::paintPathOccupied(map::OccupancyGrid& grid,
                                                const core::Path& path,
                                                int inflate_cells)
{
    if (path.empty() || grid.rows() == 0) {
        return;
    }

    for (const core::Waypoint& waypoint : path.waypoints()) {
        const auto cell = grid.worldToCell(waypoint.x, waypoint.y);
        for (int dr = -inflate_cells; dr <= inflate_cells; ++dr) {
            for (int dc = -inflate_cells; dc <= inflate_cells; ++dc) {
                const int row = cell.row + dr;
                const int col = cell.col + dc;
                if (grid.isInside(row, col)) {
                    grid.setOccupied(row, col, true);
                }
            }
        }
    }
}

}  // namespace fleetsim::domain::collision
