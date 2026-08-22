#include "GreedyAssigner.h"

#include <cmath>
#include <limits>

namespace fleetsim::domain::scheduling {

namespace {

double distanceToPickup(const core::VehicleState& vehicle, const core::Task& task)
{
    const double dx = task.pickup.x - vehicle.pose.x;
    const double dy = task.pickup.y - vehicle.pose.y;
    return std::sqrt(dx * dx + dy * dy);
}

}  // namespace

std::vector<TaskAssignment> GreedyAssigner::assign(
    const std::vector<core::Task>& pending_tasks,
    const std::vector<core::VehicleState>& idle_vehicles) const
{
    std::vector<TaskAssignment> assignments;
    if (pending_tasks.empty() || idle_vehicles.empty()) {
        return assignments;
    }

    std::vector<core::Task> remaining_tasks = pending_tasks;
    std::vector<core::VehicleState> remaining_vehicles = idle_vehicles;

    while (!remaining_tasks.empty() && !remaining_vehicles.empty()) {
        double best_distance = std::numeric_limits<double>::max();
        std::size_t best_task_index = 0;
        std::size_t best_vehicle_index = 0;

        for (std::size_t t = 0; t < remaining_tasks.size(); ++t) {
            for (std::size_t v = 0; v < remaining_vehicles.size(); ++v) {
                const double distance = distanceToPickup(remaining_vehicles[v], remaining_tasks[t]);
                if (distance < best_distance) {
                    best_distance = distance;
                    best_task_index = t;
                    best_vehicle_index = v;
                }
            }
        }

        assignments.push_back({
            remaining_tasks[best_task_index].id,
            remaining_vehicles[best_vehicle_index].id,
        });

        remaining_tasks.erase(remaining_tasks.begin() + static_cast<std::ptrdiff_t>(best_task_index));
        remaining_vehicles.erase(remaining_vehicles.begin() + static_cast<std::ptrdiff_t>(best_vehicle_index));
    }

    return assignments;
}

}  // namespace fleetsim::domain::scheduling
