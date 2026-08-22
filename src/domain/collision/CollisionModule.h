#pragma once

#include "TimeWindowCollisionAvoidance.h"
#include "vehicle/FleetManager.h"

namespace fleetsim::domain::collision {

class CollisionModule {
public:
    void clearReservations();
    void reservePath(const core::VehicleId& id,
                     const core::Path& path,
                     double start_time_s,
                     int priority,
                     const map::OccupancyGrid& grid);

    void tick(double dt, vehicle::FleetManager& fleet, double sim_time_s, const map::OccupancyGrid& grid);

    int conflictCount() const { return conflicts_; }

    const TimeWindowCollisionAvoidance& avoidance() const { return avoidance_; }

private:
    TimeWindowCollisionAvoidance avoidance_;
    int conflicts_{0};
};

}  // namespace fleetsim::domain::collision
