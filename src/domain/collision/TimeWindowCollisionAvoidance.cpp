#include "TimeWindowCollisionAvoidance.h"

#include "core/types/Waypoint.h"

#include <cmath>

namespace fleetsim::domain::collision {

void TimeWindowCollisionAvoidance::clearReservations()
{
    table_.clear();
}

void TimeWindowCollisionAvoidance::reservePath(const core::VehicleId& id,
                                               const core::Path& path,
                                               double start_time_s,
                                               int priority,
                                               const map::OccupancyGrid& grid)
{
    if (path.empty()) {
        return;
    }

    double traveled_m = 0.0;
    const auto& waypoints = path.waypoints();
    for (std::size_t i = 0; i < waypoints.size(); ++i) {
        if (i > 0) {
            const double dx = waypoints[i].x - waypoints[i - 1].x;
            const double dy = waypoints[i].y - waypoints[i - 1].y;
            traveled_m += std::sqrt(dx * dx + dy * dy);
        }

        const double arrival_time = start_time_s + traveled_m / nominal_velocity_mps_;
        core::TimeWindow window;
        window.start_s = arrival_time;
        window.end_s = arrival_time + slot_duration_s_;

        const core::GridCell cell = grid.worldToCell(waypoints[i].x, waypoints[i].y);
        if (grid.isInside(cell.row, cell.col)) {
            table_.addReservation(cell.row, cell.col, id, window, priority);
        }
    }
}

double TimeWindowCollisionAvoidance::adjustVelocityScale(
    const core::VehicleId& id,
    const core::Pose& pose,
    const core::Path& path,
    double current_time_s,
    int priority,
    const map::OccupancyGrid& grid,
    const std::vector<NearbyVehicleState>& /*nearby*/) const
{
    if (path.empty() || grid.rows() == 0) {
        return 1.0;
    }

    core::TimeWindow window;
    window.start_s = current_time_s;
    window.end_s = current_time_s + slot_duration_s_;

    const core::GridCell pose_cell = grid.worldToCell(pose.x, pose.y);
    if (grid.isInside(pose_cell.row, pose_cell.col)) {
        const double scale = table_.speedScaleFor(
            pose_cell.row, pose_cell.col, window, id, priority, conflict_slowdown_);
        if (scale < 1.0) {
            return scale;
        }
    }

    for (const core::Waypoint& waypoint : path.waypoints()) {
        const double dx = waypoint.x - pose.x;
        const double dy = waypoint.y - pose.y;
        const double distance = std::sqrt(dx * dx + dy * dy);
        if (distance > 1.5) {
            continue;
        }

        const core::GridCell cell = grid.worldToCell(waypoint.x, waypoint.y);
        if (!grid.isInside(cell.row, cell.col)) {
            continue;
        }

        const double scale = table_.speedScaleFor(
            cell.row, cell.col, window, id, priority, conflict_slowdown_);
        if (scale < 1.0) {
            return scale;
        }
    }

    return 1.0;
}

}  // namespace fleetsim::domain::collision
