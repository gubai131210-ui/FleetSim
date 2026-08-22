#pragma once

#include "ICollisionAvoidance.h"
#include "PathReservationTable.h"

namespace fleetsim::domain::collision {

class TimeWindowCollisionAvoidance : public ICollisionAvoidance {
public:
    void setSlotDuration(double slot_duration_s) { slot_duration_s_ = slot_duration_s; }
    void setNominalVelocity(double nominal_velocity_mps) { nominal_velocity_mps_ = nominal_velocity_mps; }
    void setConflictSlowdown(double scale) { conflict_slowdown_ = scale; }

    void reservePath(const core::VehicleId& id,
                     const core::Path& path,
                     double start_time_s,
                     int priority,
                     const map::OccupancyGrid& grid) override;

    double adjustVelocityScale(const core::VehicleId& id,
                               const core::Pose& pose,
                               const core::Path& path,
                               double current_time_s,
                               int priority,
                               const map::OccupancyGrid& grid,
                               const std::vector<NearbyVehicleState>& nearby) const override;

    void clearReservations() override;

    const PathReservationTable& table() const { return table_; }

private:
    PathReservationTable table_;
    double slot_duration_s_{0.5};
    double nominal_velocity_mps_{0.5};
    double conflict_slowdown_{0.3};
};

}  // namespace fleetsim::domain::collision
