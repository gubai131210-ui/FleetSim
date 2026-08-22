#include "domain/control/StanleyTracker.h"
#include "domain/vehicle/BicycleModel.h"

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

using fleetsim::core::ControlCommand;
using fleetsim::core::Path;
using fleetsim::core::Pose;
using fleetsim::core::Waypoint;
using fleetsim::domain::control::StanleyTracker;
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

double crossTrackAtFrontAxle(const Pose& pose, double wheelbase, const Path& path)
{
    const double fx = pose.x + wheelbase * std::cos(pose.theta);
    const double fy = pose.y + wheelbase * std::sin(pose.theta);
    double best = 1e9;
    const auto& w = path.waypoints();
    for (std::size_t i = 0; i + 1 < w.size(); ++i) {
        const double ax = w[i].x;
        const double ay = w[i].y;
        const double bx = w[i + 1].x;
        const double by = w[i + 1].y;
        const double abx = bx - ax;
        const double aby = by - ay;
        const double apx = fx - ax;
        const double apy = fy - ay;
        const double ab2 = abx * abx + aby * aby;
        if (ab2 < 1e-12) {
            continue;
        }
        double t = (apx * abx + apy * aby) / ab2;
        t = std::max(0.0, std::min(1.0, t));
        const double qx = ax + t * abx;
        const double qy = ay + t * aby;
        const double ex = fx - qx;
        const double ey = fy - qy;
        // Signed lateral: cross with path tangent.
        const double tang_len = std::sqrt(ab2);
        const double signed_e = (abx * ey - aby * ex) / tang_len;
        if (std::abs(signed_e) < std::abs(best)) {
            best = signed_e;
        }
    }
    return best;
}

}  // namespace

TEST(StanleyTrackerTest, SofteningEpsilonProtectsZeroSpeed)
{
    StanleyTracker tracker(2.0, 0.1, 0.6, 0.8, 0.0);
    const Path path = makeStraightPath(0.0, 5.0, 0.0);
    Pose pose{0.0, 0.3, 0.0};

    const ControlCommand cmd = tracker.compute(pose, path, 0.05);
    EXPECT_TRUE(std::isfinite(cmd.steering_angle));
    EXPECT_TRUE(std::isfinite(cmd.linear_velocity));
    // Non-zero steer expected for nonzero cross-track (ε softens v=0).
    EXPECT_NE(cmd.steering_angle, 0.0);
}

TEST(StanleyTrackerTest, SteeringClampedToMax)
{
    StanleyTracker tracker(50.0, 0.1, 0.4, 0.8, 0.5);
    const Path path = makeStraightPath(0.0, 10.0, 0.0);
    Pose pose{0.0, 2.0, 0.0};

    const ControlCommand cmd = tracker.compute(pose, path, 0.05);
    ASSERT_NE(cmd.steering_angle, 0.0);
    EXPECT_LE(std::abs(cmd.steering_angle), 0.4 + 1e-9);
}

TEST(StanleyTrackerTest, CrossTrackErrorDecreasesOnStraightPath)
{
    // Classic Stanley + bicycle integrate: |e| should trend down (RED on stub).
    const double L = 0.8;
    const double max_steer = 0.6;
    StanleyTracker tracker(2.0, 0.1, max_steer, L, 0.5);
    BicycleModel model(L, 1.0, max_steer);
    const Path path = makeStraightPath(0.0, 12.0, 0.0);

    Pose pose{0.0, 0.5, 0.05};
    const double e0 = std::abs(crossTrackAtFrontAxle(pose, L, path));

    for (int i = 0; i < 120; ++i) {
        const ControlCommand cmd = tracker.compute(pose, path, 0.05);
        pose = model.integrate(pose, cmd, 0.05);
    }

    const double e1 = std::abs(crossTrackAtFrontAxle(pose, L, path));
    EXPECT_GT(e0, 0.2);
    EXPECT_LT(e1, e0 * 0.5);
}

TEST(StanleyTrackerTest, FormulaIncludesHeadingAndCrossTrackTerms)
{
    // Place front axle on path with heading error so e≈0 and δ≈θe.
    const double L = 0.8;
    const double theta_e = 0.35;
    StanleyTracker tracker(1.5, 0.1, 0.6, L, 0.5);
    const Path path = makeStraightPath(0.0, 8.0, 0.0);
    const double fx = 2.0;
    const double fy = 0.0;
    Pose pose{
        fx - L * std::cos(theta_e),
        fy - L * std::sin(theta_e),
        theta_e};

    const ControlCommand cmd = tracker.compute(pose, path, 0.05);
    ASSERT_NE(cmd.steering_angle, 0.0);
    EXPECT_NEAR(cmd.steering_angle, theta_e, 0.15)
        << "δ ≈ θe when front-axle e≈0 (classic Stanley); PurePursuit-skin would differ";
}
