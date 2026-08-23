#include "domain/control/MpcLateralTracker.h"
#include "domain/control/StanleyTracker.h"

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

using fleetsim::core::ControlCommand;
using fleetsim::core::Path;
using fleetsim::core::Pose;
using fleetsim::core::Waypoint;
using fleetsim::domain::control::MpcLateralTracker;
using fleetsim::domain::control::StanleyTracker;

namespace {

Path makeStraightPath(double x0, double x1, double y, double step = 0.25)
{
    std::vector<Waypoint> pts;
    for (double x = x0; x <= x1 + 1e-9; x += step) {
        pts.push_back({x, y});
    }
    return Path(pts);
}

}  // namespace

TEST(MpcVsStanleyCompareTest, SamePathSteeringSequencesDiffer)
{
    // Same geometry / pose / path: MPC and Stanley must not produce identical δ
    // sequences (anti skinning). Does NOT require MPC to be everywhere better.
    const double L = 0.8;
    const double max_steer = 0.6;
    const double v = 0.5;
    const double dt = 0.05;
    StanleyTracker stanley(1.5, 0.1, max_steer, L, v);
    MpcLateralTracker mpc(10, dt, 2.0, 2.0, 0.5, max_steer, L, v);

    const Path path = makeStraightPath(0.0, 10.0, 0.0);
    Pose pose{0.0, 0.45, 0.08};

    double max_abs_diff = 0.0;
    for (int i = 0; i < 8; ++i) {
        const ControlCommand cs = stanley.compute(pose, path, dt);
        const ControlCommand cm = mpc.compute(pose, path, dt);
        max_abs_diff =
            std::max(max_abs_diff, std::abs(cs.steering_angle - cm.steering_angle));
    }

    EXPECT_GT(max_abs_diff, 0.02)
        << "Expected distinguishable steering; identical sequences suggest a skin";
}

TEST(MpcVsStanleyCompareTest, MpcSolveOkAndNonTrivialCostOnOffsetStart)
{
    MpcLateralTracker mpc(10, 0.1, 2.0, 2.0, 0.5, 0.6, 0.8, 0.5);
    const Path path = makeStraightPath(0.0, 8.0, 0.0);
    Pose pose{1.0, 0.35, 0.0};

    const ControlCommand cmd = mpc.compute(pose, path, 0.1);
    EXPECT_TRUE(mpc.lastSolveOk());
    EXPECT_GT(mpc.lastPredictionNorm(), 1e-6);
    EXPECT_TRUE(mpc.lastCostNonTrivial());
    EXPECT_NE(cmd.steering_angle, 0.0);
}
