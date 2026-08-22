#pragma once

#include "core/types/ControlCommand.h"
#include "core/types/Pose.h"

namespace fleetsim::domain::vehicle {

class DiffDriveModel {
public:
    DiffDriveModel(double max_linear_velocity = 0.5, double max_angular_velocity = 1.0);

    core::Pose integrate(const core::Pose& current_pose,
                         const core::ControlCommand& command,
                         double dt) const;

private:
    double max_linear_velocity_{0.5};
    double max_angular_velocity_{1.0};
};

}  // namespace fleetsim::domain::vehicle
