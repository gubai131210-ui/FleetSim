#pragma once

#include "core/types/Path.h"
#include "core/types/Pose.h"
#include "core/types/VehicleId.h"
#include "domain/map/OccupancyGrid.h"

#include <vector>

namespace fleetsim::domain::collision {

struct NearbyVehicleState {
    core::VehicleId id;
    core::Pose pose;
    int priority{0};
};

class ICollisionAvoidance {
public:
    virtual ~ICollisionAvoidance() = default;

    virtual void reservePath(const core::VehicleId& id,
                             const core::Path& path,
                             double start_time_s,
                             int priority,
                             const map::OccupancyGrid& grid) = 0;

    virtual double adjustVelocityScale(const core::VehicleId& id,
                                       const core::Pose& pose,
                                       const core::Path& path,
                                       double current_time_s,
                                       int priority,
                                       const map::OccupancyGrid& grid,
                                       const std::vector<NearbyVehicleState>& nearby) const = 0;

    virtual void clearReservations() = 0;
};

}  // namespace fleetsim::domain::collision
