#include "domain/vehicle/Vehicle.h"
#include "domain/vehicle/VehicleModelFactory.h"

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

TEST(VehicleTest, IntegratesPoseWithBicycleModel)
{
    auto model = fleetsim::domain::vehicle::createVehicleModel("bicycle", 1.0, 1.0, 1.0, 0.6);
    Vehicle vehicle("car_0", 1.0, Pose{0.0, 0.0, 0.0}, std::move(model));
    fleetsim::core::ControlCommand command;
    command.linear_velocity = 1.0;
    command.steering_angle = 0.0;

    vehicle.integrate(command, 0.1);
    EXPECT_NEAR(vehicle.pose().x, 0.1, 1e-6);
    EXPECT_NEAR(vehicle.pose().theta, 0.0, 1e-6);
}
