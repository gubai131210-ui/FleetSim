#include "domain/planning/DubinsPath.h"

#include <gtest/gtest.h>

#include <cmath>

using fleetsim::core::Pose;
using fleetsim::domain::planning::DubinsPath;

TEST(DubinsPathTest, ShortestPathPositiveLengthOnForwardTurn)
{
    const Pose start{0.0, 0.0, 0.0};
    const Pose goal{4.0, 2.0, 0.5};
    const double rho = 1.5;
    const auto segs = DubinsPath::shortestPath(start, goal, rho);
    ASSERT_FALSE(segs.empty());
    EXPECT_GT(DubinsPath::length(segs), 3.0);

    const auto path = DubinsPath::sample(start, segs, rho, 0.2);
    ASSERT_GE(path.size(), 3U);
    EXPECT_NEAR(path.waypoints().front().x, start.x, 1e-6);
    EXPECT_NEAR(path.waypoints().back().x, goal.x, 0.35);
    EXPECT_NEAR(path.waypoints().back().y, goal.y, 0.35);
}

TEST(DubinsPathTest, StraightAlmostAlignedIsNearEuclidean)
{
    const Pose start{0.0, 0.0, 0.0};
    const Pose goal{5.0, 0.0, 0.0};
    const auto segs = DubinsPath::shortestPath(start, goal, 1.0);
    ASSERT_FALSE(segs.empty());
    EXPECT_NEAR(DubinsPath::length(segs), 5.0, 0.15);
}
