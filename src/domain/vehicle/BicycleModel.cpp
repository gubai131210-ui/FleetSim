#include "BicycleModel.h"

#include <algorithm>
#include <cmath>

namespace fleetsim::domain::vehicle {

BicycleModel::BicycleModel(double wheelbase_m,
                           double max_linear_velocity,
                           double max_steering_rad)
    : wheelbase_m_(wheelbase_m)
    , max_linear_velocity_(max_linear_velocity)
    , max_steering_rad_(max_steering_rad)
{
}

core::Pose BicycleModel::integrate(const core::Pose& current_pose,
                                   const core::ControlCommand& /*command*/,
                                   double /*dt*/) const
{
    // Session 0 stub: intentionally does not apply bicycle kinematics.
    // Session 1 will implement: x+=v cosθ dt, y+=v sinθ dt, θ+=(v/L) tan(δ) dt.
    // Returning unchanged pose keeps BicycleModelTest RED until Session 1.
    return current_pose;
}

}  // namespace fleetsim::domain::vehicle
