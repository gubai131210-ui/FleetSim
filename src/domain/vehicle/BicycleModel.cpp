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
                                   const core::ControlCommand& command,
                                   double dt) const
{
    // Rear-axle kinematic bicycle: ẋ=v cosθ, ẏ=v sinθ, θ̇=(v/L) tanδ.
    // Ignores angular_velocity (DiffDrive-only field).
    if (dt == 0.0) {
        return current_pose;
    }

    const double linear_velocity = std::clamp(command.linear_velocity,
                                              -max_linear_velocity_,
                                              max_linear_velocity_);
    const double steering_angle = std::clamp(command.steering_angle,
                                             -max_steering_rad_,
                                             max_steering_rad_);

    core::Pose next = current_pose;
    next.x += linear_velocity * std::cos(current_pose.theta) * dt;
    next.y += linear_velocity * std::sin(current_pose.theta) * dt;

    // Protect wheelbase <= 0: no yaw rate (avoid /0 and NaN).
    if (wheelbase_m_ > 0.0) {
        next.theta += (linear_velocity / wheelbase_m_) * std::tan(steering_angle) * dt;
    }

    return next;
}

}  // namespace fleetsim::domain::vehicle
