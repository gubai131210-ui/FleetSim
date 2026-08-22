#include "DiffDriveModel.h"

#include <algorithm>
#include <cmath>

namespace fleetsim::domain::vehicle {

DiffDriveModel::DiffDriveModel(double max_linear_velocity, double max_angular_velocity)
    : max_linear_velocity_(max_linear_velocity)
    , max_angular_velocity_(max_angular_velocity)
{
}

core::Pose DiffDriveModel::integrate(const core::Pose& current_pose,
                                     const core::ControlCommand& command,
                                     double dt) const
{
    const double linear_velocity = std::clamp(command.linear_velocity,
                                              -max_linear_velocity_,
                                              max_linear_velocity_);
    const double angular_velocity = std::clamp(command.angular_velocity,
                                               -max_angular_velocity_,
                                               max_angular_velocity_);

    core::Pose next = current_pose;
    next.theta += angular_velocity * dt;
    next.x += linear_velocity * std::cos(current_pose.theta) * dt;
    next.y += linear_velocity * std::sin(current_pose.theta) * dt;
    return next;
}

}  // namespace fleetsim::domain::vehicle
