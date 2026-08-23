#include "domain/control/MpcLateralTracker.h"
#include "domain/vehicle/BicycleModel.h"

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

using fleetsim::core::ControlCommand;
using fleetsim::core::Path;
using fleetsim::core::Pose;
using fleetsim::core::SpeedProfile;
using fleetsim::core::Waypoint;
using fleetsim::domain::control::MpcLateralTracker;
using fleetsim::domain::vehicle::BicycleModel;

namespace {

Path makeStraightPath(double x0, double x1, double y, double step = 0.2)
{
    std::vector<Waypoint> pts;
    for (double x = x0; x <= x1 + 1e-9; x += step) {
        pts.push_back({x, y});
    }
    return Path(pts);
}

double crossTrackAtRear(const Pose& pose, const Path& path)
{
    double best = 1e9;
    const auto& w = path.waypoints();
    for (std::size_t i = 0; i + 1 < w.size(); ++i) {
        const double ax = w[i].x;
        const double ay = w[i].y;
        const double bx = w[i + 1].x;
        const double by = w[i + 1].y;
        const double abx = bx - ax;
        const double aby = by - ay;
        const double apx = pose.x - ax;
        const double apy = pose.y - ay;
        const double ab2 = abx * abx + aby * aby;
        if (ab2 < 1e-12) {
            continue;
        }
        double t = (apx * abx + apy * aby) / ab2;
        t = std::max(0.0, std::min(1.0, t));
        const double qx = ax + t * abx;
        const double qy = ay + t * aby;
        const double ex = pose.x - qx;
        const double ey = pose.y - qy;
        const double tang_len = std::sqrt(ab2);
        const double signed_e = (abx * ey - aby * ex) / tang_len;
        if (std::abs(signed_e) < std::abs(best)) {
            best = signed_e;
        }
    }
    return best;
}

}  // namespace

TEST(MpcLateralTrackerTest, LowSpeedNoNan)
{
    MpcLateralTracker tracker(10, 0.1, 2.0, 2.0, 0.5, 0.6, 0.8, 0.0);
    const Path path = makeStraightPath(0.0, 5.0, 0.0);
    Pose pose{0.0, 0.3, 0.0};

    const ControlCommand cmd = tracker.compute(pose, path, 0.05);
    EXPECT_TRUE(std::isfinite(cmd.steering_angle));
    EXPECT_TRUE(std::isfinite(cmd.linear_velocity));
}

TEST(MpcLateralTrackerTest, SteeringClampedToMax)
{
    // Real MPC with large cross-track must clamp; stub returns 0 → FAIL until Session 1.
    MpcLateralTracker tracker(10, 0.1, 50.0, 50.0, 0.01, 0.4, 0.8, 0.5);
    const Path path = makeStraightPath(0.0, 10.0, 0.0);
    Pose pose{0.0, 2.0, 0.0};

    const ControlCommand cmd = tracker.compute(pose, path, 0.05);
    ASSERT_NE(cmd.steering_angle, 0.0) << "Session0 stub returns zero steer; Session1 QP required";
    EXPECT_LE(std::abs(cmd.steering_angle), 0.4 + 1e-9);
}

TEST(MpcLateralTrackerTest, CrossTrackErrorDecreasesOnStraightPath)
{
    const double L = 0.8;
    const double max_steer = 0.6;
    MpcLateralTracker tracker(10, 0.1, 5.0, 5.0, 0.2, max_steer, L, 0.5);
    BicycleModel model(L, 1.0, max_steer);
    const Path path = makeStraightPath(0.0, 12.0, 0.0);

    Pose pose{0.0, 0.5, 0.05};
    const double e0 = std::abs(crossTrackAtRear(pose, path));

    for (int i = 0; i < 120; ++i) {
        const ControlCommand cmd = tracker.compute(pose, path, 0.05);
        pose = model.integrate(pose, cmd, 0.05);
    }

    const double e1 = std::abs(crossTrackAtRear(pose, path));
    EXPECT_GT(e0, 0.2);
    EXPECT_LT(e1, e0 * 0.5) << "Session0 stub does not correct cross-track; Session1 required";
}

TEST(MpcLateralTrackerTest, PredictionOrCostNonTrivial)
{
    // Session 1 must expose non-zero prediction / cost structure.
    // Stub has no A/B or cost → this anchors anti-skinning (M31/M33).
    MpcLateralTracker tracker(10, 0.1, 2.0, 2.0, 0.5, 0.6, 0.8, 0.5);
    const Path path = makeStraightPath(0.0, 8.0, 0.0);
    Pose pose{1.0, 0.4, 0.0};

    const ControlCommand cmd = tracker.compute(pose, path, 0.1);
    // Non-trivial MPC: nonzero steer under lateral error OR lastSolveOk after real QP.
    const bool nontrivial =
        (std::abs(cmd.steering_angle) > 1e-6) || tracker.lastSolveOk();
    EXPECT_TRUE(nontrivial)
        << "Expected prediction/QP activity; Session0 stub has neither (red light)";
}

TEST(MpcLateralTrackerTest, QpFailureReturnsZeroSteer)
{
    // Invalid horizon forces failure path once Session 1 implements QP.
    MpcLateralTracker tracker(0, 0.1, 2.0, 2.0, 0.5, 0.6, 0.8, 0.5);
    const Path path = makeStraightPath(0.0, 5.0, 0.0);
    Pose pose{0.0, 0.2, 0.0};

    const ControlCommand cmd = tracker.compute(pose, path, 0.05);
    EXPECT_DOUBLE_EQ(cmd.steering_angle, 0.0);
    EXPECT_FALSE(tracker.lastSolveOk());
}

TEST(MpcLateralTrackerTest, UsesSpeedProfileWhenSet)
{
    MpcLateralTracker tracker(10, 0.1, 2.0, 2.0, 0.5, 0.6, 0.8, 0.5);
    const Path path = makeStraightPath(0.0, 4.0, 0.0, 1.0);
    SpeedProfile profile;
    profile.speeds.assign(path.size(), 0.25);
    profile.arrival_times.assign(path.size(), 0.0);
    for (std::size_t i = 1; i < path.size(); ++i) {
        profile.arrival_times[i] = static_cast<double>(i) * 4.0;
    }
    tracker.setSpeedProfile(&profile);

    Pose pose{0.0, 0.0, 0.0};
    const ControlCommand cmd = tracker.compute(pose, path, 0.05);
    EXPECT_NEAR(cmd.linear_velocity, 0.25, 1e-9);
}
