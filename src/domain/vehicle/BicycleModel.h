#pragma once

#include "IVehicleModel.h"

namespace fleetsim::domain::vehicle {

// Kinematic bicycle (rear-axle reference). Session 0: stub only — Session 1 implements
// ẋ=v cosθ, ẏ=v sinθ, θ̇=(v/L) tanδ.
class BicycleModel : public IVehicleModel {
public:
    BicycleModel(double wheelbase_m = 0.8,
                 double max_linear_velocity = 0.5,
                 double max_steering_rad = 0.6);

    core::Pose integrate(const core::Pose& current_pose,
                         const core::ControlCommand& command,
                         double dt) const override;

    double wheelbaseM() const { return wheelbase_m_; }
    double maxSteeringRad() const { return max_steering_rad_; }

private:
    double wheelbase_m_{0.8};
    double max_linear_velocity_{0.5};
    double max_steering_rad_{0.6};
};

}  // namespace fleetsim::domain::vehicle
