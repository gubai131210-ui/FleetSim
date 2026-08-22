#include "domain/vehicle/BicycleModel.h"

#include <cmath>
#include <gtest/gtest.h>

using fleetsim::core::ControlCommand;
using fleetsim::core::Pose;
using fleetsim::domain::vehicle::BicycleModel;

namespace {

constexpr double kEps = 1e-6;

}  // namespace

// Expected GREEN after Session 1 implements rear-axle bicycle kinematics.
// Session 0 stub returns unchanged pose → these assertions FAIL (RED).

TEST(BicycleModelTest, StraightLineIncreasesX)
{
    BicycleModel model(1.0, 1.0, 0.6);
    Pose pose{0.0, 0.0, 0.0};
    ControlCommand cmd;
    cmd.linear_velocity = 1.0;
    cmd.steering_angle = 0.0;

    const Pose next = model.integrate(pose, cmd, 0.1);
    EXPECT_NEAR(next.x, 0.1, kEps);
    EXPECT_NEAR(next.y, 0.0, kEps);
    EXPECT_NEAR(next.theta, 0.0, kEps);
}

TEST(BicycleModelTest, ConstantSteeringProducesYawRate)
{
    // θ̇ = (v/L) tan(δ); L=1, v=1, δ=π/4 → θ̇ = tan(π/4) = 1
    BicycleModel model(1.0, 2.0, 1.0);
    Pose pose{0.0, 0.0, 0.0};
    ControlCommand cmd;
    cmd.linear_velocity = 1.0;
    cmd.steering_angle = 0.7853981633974483;  // π/4

    const Pose next = model.integrate(pose, cmd, 0.1);
    EXPECT_NEAR(next.theta, 0.1, 1e-4);
    EXPECT_GT(next.x, 0.0);
}

TEST(BicycleModelTest, SteeringIsClamped)
{
    BicycleModel model(1.0, 1.0, 0.3);
    Pose pose{0.0, 0.0, 0.0};
    ControlCommand cmd;
    cmd.linear_velocity = 1.0;
    cmd.steering_angle = 1.0;  // above max 0.3

    const Pose next = model.integrate(pose, cmd, 0.1);
    const double expected_theta = 0.1 * std::tan(0.3);
    EXPECT_NEAR(next.theta, expected_theta, 1e-5);
}

TEST(BicycleModelTest, ZeroVelocityKeepsPose)
{
    BicycleModel model(1.0, 1.0, 0.6);
    Pose pose{1.0, 2.0, 0.5};
    ControlCommand cmd;
    cmd.linear_velocity = 0.0;
    cmd.steering_angle = 0.4;

    const Pose next = model.integrate(pose, cmd, 0.1);
    EXPECT_NEAR(next.x, pose.x, kEps);
    EXPECT_NEAR(next.y, pose.y, kEps);
    EXPECT_NEAR(next.theta, pose.theta, kEps);
}

TEST(BicycleModelTest, DoesNotUseAngularVelocityLikeDiffDrive)
{
    // If mistakenly implemented as DiffDrive, ω=1 would yaw without steering.
    BicycleModel model(1.0, 1.0, 0.6);
    Pose pose{0.0, 0.0, 0.0};
    ControlCommand cmd;
    cmd.linear_velocity = 0.0;
    cmd.angular_velocity = 1.0;
    cmd.steering_angle = 0.0;

    const Pose next = model.integrate(pose, cmd, 0.1);
    EXPECT_NEAR(next.theta, 0.0, kEps);
}
