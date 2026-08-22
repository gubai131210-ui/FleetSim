#pragma once

namespace fleetsim::core {

struct ControlCommand {
    double linear_velocity{0.0};
    double angular_velocity{0.0};
};

}  // namespace fleetsim::core
