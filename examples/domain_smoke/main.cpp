// No-Qt smoke: integrate BicycleModel once and print pose.
#include "domain/vehicle/BicycleModel.h"
#include "core/types/ControlCommand.h"
#include "core/types/Pose.h"

#include <cstdio>

int main()
{
    fleetsim::domain::vehicle::BicycleModel model(0.9, 1.0, 0.6);
    fleetsim::core::Pose pose{0.0, 0.0, 0.0};
    fleetsim::core::ControlCommand cmd;
    cmd.linear_velocity = 0.5;
    cmd.steering_angle = 0.2;

    pose = model.integrate(pose, cmd, 0.1);
    std::printf("domain_smoke ok pose=(%.4f, %.4f, %.4f)\n", pose.x, pose.y, pose.theta);
    return (pose.x > 0.0) ? 0 : 1;
}
