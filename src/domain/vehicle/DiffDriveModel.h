#pragma once

#include "IVehicleModel.h"

namespace fleetsim::domain::vehicle {

class DiffDriveModel : public IVehicleModel {
public:
    DiffDriveModel(double max_linear_velocity = 0.5, double max_angular_velocity = 1.0);

    core::Pose integrate(const core::Pose& current_pose,
                         const core::ControlCommand& command,
                         double dt) const override;

private:
    double max_linear_velocity_{0.5};
    double max_angular_velocity_{1.0};
};

}  // namespace fleetsim::domain::vehicle
