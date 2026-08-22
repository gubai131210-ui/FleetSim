#include "Vehicle.h"

namespace fleetsim::domain::vehicle {

Vehicle::Vehicle(core::VehicleId id, double length_m, const core::Pose& initial_pose)
    : id_(std::move(id))
    , length_m_(length_m)
    , pose_(initial_pose)
{
}

void Vehicle::integrate(const core::ControlCommand& command, double dt)
{
    pose_ = model_.integrate(pose_, command, dt);
}

}  // namespace fleetsim::domain::vehicle
