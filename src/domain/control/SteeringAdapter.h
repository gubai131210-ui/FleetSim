#pragma once

#include "core/types/ControlCommand.h"

namespace fleetsim::domain::control {

// Pure Pursuit curvature κ → bicycle steering δ = atan(κ * L).
class SteeringAdapter {
public:
    // Returns clamped steering angle (rad). wheelbase_m <= 0 → 0.
    static double steeringFromCurvature(double curvature,
                                        double wheelbase_m,
                                        double max_steering_rad = 0.6);

    // Fills command.steering_angle from angular_velocity / linear_velocity when v≈0 uses κ=0.
    // Prefer steeringFromCurvature when κ is known explicitly.
    static void applyCurvatureToCommand(core::ControlCommand& command,
                                        double curvature,
                                        double wheelbase_m,
                                        double max_steering_rad = 0.6);
};

}  // namespace fleetsim::domain::control
