#include "domain/vehicle/Vehicle.h"

#include <gtest/gtest.h>

using fleetsim::core::Pose;
using fleetsim::domain::vehicle::Vehicle;

TEST(VehicleTest, IntegratesPoseWithDiffDrive)
{
    Vehicle vehicle("agv_0", 1.0, Pose{0.0, 0.0, 0.0});
    fleetsim::core::ControlCommand command;
    command.linear_velocity = 0.4;
    command.angular_velocity = 0.0;

    vehicle.integrate(command, 0.05);
    EXPECT_GT(vehicle.pose().x, 0.0);
}
