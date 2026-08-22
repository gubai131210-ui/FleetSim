#include "CollisionModule.h"

namespace fleetsim::domain::collision {

void CollisionModule::clearReservations()
{
    avoidance_.clearReservations();
    conflicts_ = 0;
}

void CollisionModule::reservePath(const core::VehicleId& id,
                                  const core::Path& path,
                                  double start_time_s,
                                  int priority,
                                  const map::OccupancyGrid& grid)
{
    avoidance_.reservePath(id, path, start_time_s, priority, grid);
}

void CollisionModule::tick(double dt,
                           vehicle::FleetManager& fleet,
                           double sim_time_s,
                           const map::OccupancyGrid& grid)
{
    (void)dt;
    conflicts_ = 0;

    for (std::size_t i = 0; i < fleet.count(); ++i) {
        vehicle::VehicleAgent& agent = fleet.agent(i);
        if (agent.reference_path.empty()) {
            agent.speed_scale = 1.0;
            continue;
        }

        std::vector<NearbyVehicleState> nearby;
        for (std::size_t j = 0; j < fleet.count(); ++j) {
            if (i == j) {
                continue;
            }
            const vehicle::VehicleAgent& other = fleet.agent(j);
            nearby.push_back({
                other.vehicle->id(),
                other.vehicle->pose(),
                other.task_priority,
            });
        }

        const double scale = avoidance_.adjustVelocityScale(
            agent.vehicle->id(),
            agent.vehicle->pose(),
            agent.reference_path,
            sim_time_s,
            agent.task_priority,
            grid,
            nearby);

        agent.speed_scale = scale;
        if (scale < 1.0) {
            ++conflicts_;
        }
    }
}

}  // namespace fleetsim::domain::collision
