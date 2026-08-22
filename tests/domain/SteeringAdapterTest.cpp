#include "domain/control/SteeringAdapter.h"
#include "domain/control/PurePursuitTracker.h"
#include "domain/vehicle/BicycleModel.h"

#include <cmath>
#include <gtest/gtest.h>
#include <vector>

using fleetsim::core::ControlCommand;
using fleetsim::core::Path;
using fleetsim::core::Pose;
using fleetsim::core::Waypoint;
using fleetsim::domain::control::PurePursuitTracker;
using fleetsim::domain::control::SteeringAdapter;
using fleetsim::domain::vehicle::BicycleModel;

TEST(SteeringAdapterTest, CurvatureMapsToSteeringAngle)
{
    // δ = atan(κ L); κ=0.5, L=1 → δ=atan(0.5)
    const double steering = SteeringAdapter::steeringFromCurvature(0.5, 1.0, 1.0);
    EXPECT_NEAR(steering, std::atan(0.5), 1e-9);
}

TEST(SteeringAdapterTest, ClampsSteering)
{
    const double steering = SteeringAdapter::steeringFromCurvature(10.0, 1.0, 0.4);
    EXPECT_NEAR(steering, 0.4, 1e-9);
}

TEST(SteeringAdapterTest, ZeroWheelbaseYieldsZeroSteering)
{
    EXPECT_NEAR(SteeringAdapter::steeringFromCurvature(1.0, 0.0, 0.6), 0.0, 1e-12);
}

TEST(PurePursuitTrackerTest, BicycleModeFillsSteeringAngle)
{
    std::vector<Waypoint> points;
    for (int i = 0; i <= 30; ++i) {
        points.push_back({static_cast<double>(i) * 0.2, 0.0});
    }
    // Gentle left curve ahead
    points.push_back({6.5, 0.5});
    points.push_back({7.0, 1.0});

    Path path(points);
    PurePursuitTracker tracker(0.8, 0.5, 1.0, 0.785, 0.2);
    Pose pose{0.0, 0.0, 0.0};

    const ControlCommand cmd = tracker.compute(pose, path, 0.05, 0.9, 0.6);
    EXPECT_GT(cmd.linear_velocity, 0.0);
    // Straight-ish early path → small steering; still must be finite and clamped
    EXPECT_LE(std::abs(cmd.steering_angle), 0.6 + 1e-9);
    EXPECT_TRUE(std::isfinite(cmd.steering_angle));
}

TEST(PurePursuitTrackerTest, BicycleAndModelTurnSameSign)
{
    // Constant left steering command through bicycle model should increase theta.
    BicycleModel model(1.0, 1.0, 0.6);
    Pose pose{0.0, 0.0, 0.0};
    ControlCommand cmd;
    cmd.linear_velocity = 0.5;
    cmd.steering_angle = SteeringAdapter::steeringFromCurvature(0.5, 1.0, 0.6);

    for (int i = 0; i < 20; ++i) {
        pose = model.integrate(pose, cmd, 0.05);
    }
    EXPECT_GT(pose.theta, 0.0);
    EXPECT_GT(pose.x, 0.0);
}
