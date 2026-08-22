#include "domain/planning/AStarPlanner.h"
#include "domain/planning/DouglasPeuckerSmoother.h"
#include "domain/map/MapLoader.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <vector>

using fleetsim::core::Pose;
using fleetsim::domain::map::MapLoader;
using fleetsim::domain::planning::AStarPlanner;
using fleetsim::domain::planning::DouglasPeuckerSmoother;

namespace {

std::string demoMapPath()
{
    const std::filesystem::path source_dir = std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
    return (source_dir / "assets" / "scenarios" / "demo" / "map.json").string();
}

}  // namespace

TEST(AStarPlannerTest, FindsPathAroundObstacle)
{
    auto grid = MapLoader::loadFromFile(demoMapPath(), 0.55);
    AStarPlanner planner;

    const Pose start{2.0, 2.0, 0.0};
    const Pose goal{18.0, 12.0, 0.0};
    const auto path = planner.plan(grid, start, goal);

    EXPECT_FALSE(path.empty());
    EXPECT_GT(path.size(), 2U);
}

TEST(DouglasPeuckerSmootherTest, ReducesWaypointsButKeepsEndpoints)
{
    std::vector<fleetsim::core::Waypoint> zigzag;
    for (int i = 0; i < 20; ++i) {
        zigzag.push_back({static_cast<double>(i), static_cast<double>(i % 2)});
    }

    fleetsim::core::Path raw(zigzag);
    DouglasPeuckerSmoother smoother;
    const auto smoothed = smoother.smooth(raw);

    EXPECT_GE(smoothed.size(), 2U);
    EXPECT_LT(smoothed.size(), raw.size());
    EXPECT_DOUBLE_EQ(smoothed.waypoints().front().x, raw.waypoints().front().x);
    EXPECT_DOUBLE_EQ(smoothed.waypoints().back().x, raw.waypoints().back().x);
}
