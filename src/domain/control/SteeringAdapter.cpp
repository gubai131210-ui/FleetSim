#include "SteeringAdapter.h"

#include <algorithm>
#include <cmath>

namespace fleetsim::domain::control {

double SteeringAdapter::steeringFromCurvature(double curvature,
                                              double wheelbase_m,
                                              double max_steering_rad)
{
    if (wheelbase_m <= 0.0 || max_steering_rad <= 0.0) {
        return 0.0;
    }

    const double steering = std::atan(curvature * wheelbase_m);
    return std::clamp(steering, -max_steering_rad, max_steering_rad);
}

void SteeringAdapter::applyCurvatureToCommand(core::ControlCommand& command,
                                              double curvature,
                                              double wheelbase_m,
                                              double max_steering_rad)
{
    command.steering_angle =
        steeringFromCurvature(curvature, wheelbase_m, max_steering_rad);
}

}  // namespace fleetsim::domain::control
