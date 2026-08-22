#pragma once

namespace fleetsim::core {

struct ControlCommand {
    double linear_velocity{0.0};
    double angular_velocity{0.0};
    // Bicycle uses steering_angle (rad); DiffDrive ignores it.
    double steering_angle{0.0};
};

}  // namespace fleetsim::core
