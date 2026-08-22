#include "domain/planning/HybridAStarPlanner.h"
#include "domain/planning/AStarPlanner.h"
#include "domain/map/OccupancyGrid.h"

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

using fleetsim::core::Path;
using fleetsim::core::Pose;
using fleetsim::core::Waypoint;
using fleetsim::domain::map::OccupancyGrid;
using fleetsim::domain::planning::AStarPlanner;
using fleetsim::domain::planning::HybridAStarPlanner;

namespace {

OccupancyGrid makeOpenGrid(double resolution = 0.25, int rows = 40, int cols = 40)
{
    const double width_m = cols * resolution;
    const double height_m = rows * resolution;
    OccupancyGrid grid(rows, cols, resolution, width_m, height_m);
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            grid.setOccupied(row, col, false);
        }
    }
    return grid;
}

/// Corridor wide enough for R_min≈0.7m bicycle turn (not a 1m knife-edge).
OccupancyGrid makeNarrowTurnCorridor()
{
    // 10m x 10m, free L-shaped corridor ~2.0m wide.
    constexpr int kRows = 100;
    constexpr int kCols = 100;
    constexpr double kRes = 0.1;
    OccupancyGrid grid(kRows, kCols, kRes, kCols * kRes, kRows * kRes);
    for (int row = 0; row < kRows; ++row) {
        for (int col = 0; col < kCols; ++col) {
            grid.setOccupied(row, col, true);
        }
    }
    // Horizontal arm: y in [1.5, 3.5] → rows 15..35; x in [0.5, 7.5] → cols 5..75
    for (int row = 15; row <= 35; ++row) {
        for (int col = 5; col <= 75; ++col) {
            grid.setOccupied(row, col, false);
        }
    }
    // Vertical arm: x in [5.5, 7.5] → cols 55..75; y in [1.5, 7.5] → rows 15..75
    for (int row = 15; row <= 75; ++row) {
        for (int col = 55; col <= 75; ++col) {
            grid.setOccupied(row, col, false);
        }
    }
    return grid;
}

double maxSegmentCurvatureProxy(const Path& path)
{
    if (path.size() < 3) {
        return 0.0;
    }
    double max_kappa = 0.0;
    const auto& w = path.waypoints();
    for (std::size_t i = 1; i + 1 < w.size(); ++i) {
        const double ax = w[i].x - w[i - 1].x;
        const double ay = w[i].y - w[i - 1].y;
        const double bx = w[i + 1].x - w[i].x;
        const double by = w[i + 1].y - w[i].y;
        const double la = std::hypot(ax, ay);
        const double lb = std::hypot(bx, by);
        if (la < 1e-6 || lb < 1e-6) {
            continue;
        }
        const double cross = ax * by - ay * bx;
        const double sin_phi = std::abs(cross) / (la * lb);
        const double kappa = sin_phi / std::max(0.5 * (la + lb), 1e-6);
        max_kappa = std::max(max_kappa, kappa);
    }
    return max_kappa;
}

}  // namespace

TEST(HybridAStarPlannerTest, MinTurningRadiusMatchesBicycleGeometry)
{
    HybridAStarPlanner planner(1.0, 0.5);
    const double expected = 1.0 / std::tan(0.5);
    EXPECT_NEAR(planner.minTurningRadiusM(), expected, 1e-9);
}

TEST(HybridAStarPlannerTest, PlansCurvedPathRespectingMinTurningRadius)
{
    // RED until Session 1: stub returns empty path.
    const OccupancyGrid grid = makeOpenGrid();
    HybridAStarPlanner planner(0.8, 0.6);
    const Pose start{1.0, 2.0, 0.0};
    const Pose goal{8.0, 2.0, 0.0};

    const Path path = planner.plan(grid, start, goal);

    ASSERT_FALSE(path.empty()) << "HybridAStarPlanner Session0 stub; expect non-empty after Session 1";
    EXPECT_GT(path.size(), 2U);
    EXPECT_NEAR(path.waypoints().front().x, start.x, 0.5);
    EXPECT_NEAR(path.waypoints().back().x, goal.x, 0.5);
}

TEST(HybridAStarPlannerTest, UsesStartAndGoalThetaNotOnlyXY)
{
    // Same XY start/goal offset but opposite headings should not yield identical
    // first-segment directions once kinematics are implemented.
    const OccupancyGrid grid = makeOpenGrid();
    HybridAStarPlanner planner(0.8, 0.6);

    const Pose start_a{2.0, 2.0, 0.0};
    const Pose goal_a{6.0, 5.0, 1.2};
    const Pose start_b{2.0, 2.0, 3.0};
    const Pose goal_b{6.0, 5.0, -1.2};

    const Path path_a = planner.plan(grid, start_a, goal_a);
    const Path path_b = planner.plan(grid, start_b, goal_b);

    ASSERT_FALSE(path_a.empty());
    ASSERT_FALSE(path_b.empty());
    ASSERT_GE(path_a.size(), 3U);
    ASSERT_GE(path_b.size(), 3U);

    const double dir_a = std::atan2(path_a.waypoints()[1].y - path_a.waypoints()[0].y,
                                    path_a.waypoints()[1].x - path_a.waypoints()[0].x);
    const double dir_b = std::atan2(path_b.waypoints()[1].y - path_b.waypoints()[0].y,
                                    path_b.waypoints()[1].x - path_b.waypoints()[0].x);
    EXPECT_GT(std::abs(dir_a - dir_b), 0.2)
        << "Fake Hybrid(=grid A*) would ignore start theta; real Hybrid must differ";
}

TEST(HybridAStarPlannerTest, NarrowTurnPrefersKinematicFeasibilityOverGridShortcut)
{
    // Contrast: Hybrid returns a path whose curvature proxy stays near 1/R_min.
    const OccupancyGrid grid = makeNarrowTurnCorridor();
    // R_min = 0.6/tan(0.7) ≈ 0.71m — fits ~2m corridor.
    HybridAStarPlanner hybrid(0.6, 0.7, 0.2, 3);
    AStarPlanner astar;

    const Pose start{1.0, 2.5, 0.0};
    const Pose goal{6.5, 6.5, 1.57};

    const Path hybrid_path = hybrid.plan(grid, start, goal);
    const Path astar_path = astar.plan(grid, start, goal);

    ASSERT_FALSE(hybrid_path.empty()) << "Hybrid must find a kinematically feasible L-turn";
    EXPECT_FALSE(astar_path.empty());

    const double r_min = hybrid.minTurningRadiusM();
    const double kappa_max = maxSegmentCurvatureProxy(hybrid_path);
    EXPECT_LE(kappa_max, 1.0 / r_min * 2.0 + 1.0)
        << "Hybrid path should roughly respect min turning radius";
}
