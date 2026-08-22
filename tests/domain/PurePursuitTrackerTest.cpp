#include "domain/control/PurePursuitTracker.h"
#include "domain/vehicle/DiffDriveModel.h"

#include <gtest/gtest.h>

#include <vector>

using fleetsim::core::Path;
using fleetsim::core::Pose;
using fleetsim::core::Waypoint;
using fleetsim::domain::control::PurePursuitTracker;
using fleetsim::domain::vehicle::DiffDriveModel;

TEST(PurePursuitTrackerTest, TracksStraightLineWithLowLateralError)
{
    std::vector<Waypoint> points;
    for (int i = 0; i <= 20; ++i) {
        points.push_back({static_cast<double>(i), 0.0});
    }

    Path path(points);
    PurePursuitTracker tracker;
    DiffDriveModel model;

    Pose pose{0.0, 0.5, 0.0};
    for (int i = 0; i < 100; ++i) {
        const auto command = tracker.compute(pose, path, 0.05);
        pose = model.integrate(pose, command, 0.05);
    }

    EXPECT_GT(pose.x, 1.0);
    EXPECT_NEAR(pose.y, 0.5, 0.15);
}

TEST(DiffDriveModelTest, IntegratesForwardMotion)
{
    DiffDriveModel model;
    Pose pose{0.0, 0.0, 0.0};
    fleetsim::core::ControlCommand command;
    command.linear_velocity = 0.5;
    command.angular_velocity = 0.0;

    pose = model.integrate(pose, command, 0.1);
    EXPECT_NEAR(pose.x, 0.05, 1e-6);
    EXPECT_NEAR(pose.y, 0.0, 1e-6);
}
