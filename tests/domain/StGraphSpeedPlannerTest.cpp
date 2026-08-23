#include "domain/planning/StGraphSpeedPlanner.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <vector>

using fleetsim::core::Path;
using fleetsim::core::SpeedProfile;
using fleetsim::core::Waypoint;
using fleetsim::domain::planning::PeerTrajectory;
using fleetsim::domain::planning::StGraphSpeedPlanner;

namespace {

Path makePath(std::initializer_list<Waypoint> pts)
{
    return Path(std::vector<Waypoint>(pts));
}

double minSpeed(const SpeedProfile& p)
{
    if (p.speeds.empty()) {
        return 0.0;
    }
    return *std::min_element(p.speeds.begin(), p.speeds.end());
}

}  // namespace

TEST(StGraphSpeedPlannerTest, SpeedProfileLengthMatchesPath)
{
    StGraphSpeedPlanner planner(0.5, 0.8, 0.1);
    const Path ego = makePath({{0, 0}, {1, 0}, {2, 0}, {3, 0}});
    const SpeedProfile profile = planner.plan(ego, {});
    EXPECT_EQ(profile.speeds.size(), ego.size());
    EXPECT_EQ(profile.arrival_times.size(), ego.size());
    if (!profile.arrival_times.empty()) {
        EXPECT_NEAR(profile.arrival_times.front(), 0.0, 1e-12);
    }
}

TEST(StGraphSpeedPlannerTest, EmptyObstaclesYieldsNearCruiseProfile)
{
    const double v_max = 0.5;
    StGraphSpeedPlanner planner(v_max, 0.8, 0.1);
    const Path ego = makePath({{0, 0}, {2, 0}, {4, 0}, {6, 0}});
    const SpeedProfile profile = planner.plan(ego, {});
    ASSERT_EQ(profile.speeds.size(), ego.size());
    for (double v : profile.speeds) {
        EXPECT_NEAR(v, v_max, 1e-6);
    }
}

TEST(StGraphSpeedPlannerTest, PeerCrossingProducesDecelerationNotDistanceStop)
{
    // Ego along x-axis; peer crosses at x=3. Real ST must slow before conflict.
    // Session0 stub ignores peers → FAIL until Session 3.
    StGraphSpeedPlanner planner(0.5, 0.8, 0.1);
    const Path ego = makePath({{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}});
    PeerTrajectory peer;
    peer.path = makePath({{3, -2}, {3, -1}, {3, 0}, {3, 1}, {3, 2}});
    peer.nominal_speed = 0.5;

    const SpeedProfile with_peer = planner.plan(ego, {peer});
    ASSERT_EQ(with_peer.speeds.size(), ego.size());

    const double v_min = minSpeed(with_peer);
    EXPECT_LT(v_min, 0.5 - 1e-3)
        << "Expected ST deceleration near crossing; stub ignores peers (red light)";

    // Anti distance-stop: must not be a single hard zero only at one cell without
    // a preceding slow-down trend (Session 3 strengthens this). Soft check:
    EXPECT_GT(v_min, -1e-9);
}

TEST(StGraphSpeedPlannerTest, ClearingPeerObstaclesChangesProfile)
{
    StGraphSpeedPlanner planner(0.5, 0.8, 0.1);
    const Path ego = makePath({{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}});
    PeerTrajectory peer;
    peer.path = makePath({{3, -2}, {3, 0}, {3, 2}});
    peer.nominal_speed = 0.4;

    const SpeedProfile with_peer = planner.plan(ego, {peer});
    const SpeedProfile without_peer = planner.plan(ego, {});

    ASSERT_EQ(with_peer.speeds.size(), without_peer.speeds.size());
    bool differs = false;
    for (std::size_t i = 0; i < with_peer.speeds.size(); ++i) {
        if (std::abs(with_peer.speeds[i] - without_peer.speeds[i]) > 1e-6) {
            differs = true;
            break;
        }
    }
    EXPECT_TRUE(differs)
        << "Clearing peers must change SpeedProfile; stub ignores peers (red light)";
}
