#include "domain/SimEngine.h"
#include "domain/scenario/ScenarioLoader.h"
#include "domain/planning/AStarPlanner.h"
#include "domain/planning/HybridAStarPlanner.h"
#include "domain/vehicle/Vehicle.h"
#include "domain/vehicle/VehicleModelFactory.h"

#include <cmath>
#include <filesystem>
#include <gtest/gtest.h>

using fleetsim::core::Pose;
using fleetsim::domain::SimEngine;
using fleetsim::domain::planning::AStarPlanner;
using fleetsim::domain::planning::HybridAStarPlanner;
using fleetsim::domain::scenario::ScenarioLoader;
using fleetsim::domain::vehicle::Vehicle;

namespace {

std::filesystem::path findScenarioDir(const char* name)
{
    const std::filesystem::path candidates[] = {
        std::filesystem::path("assets") / "scenarios" / name,
        std::filesystem::path("../assets") / "scenarios" / name,
        std::filesystem::path("../../assets") / "scenarios" / name,
        std::filesystem::path("../../../assets") / "scenarios" / name,
        std::filesystem::path("../../../../assets") / "scenarios" / name,
    };
    for (const auto& path : candidates) {
        if (std::filesystem::exists(path / "scenario.json")) {
            return path;
        }
    }
    return {};
}

bool pathsNearlyIdentical(const fleetsim::core::Path& a, const fleetsim::core::Path& b)
{
    if (a.size() != b.size()) {
        return false;
    }
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (std::abs(a.waypoints()[i].x - b.waypoints()[i].x) > 1e-3
            || std::abs(a.waypoints()[i].y - b.waypoints()[i].y) > 1e-3) {
            return false;
        }
    }
    return true;
}

}  // namespace

TEST(PlannerSwitchIntegrationTest, BicycleAutoDefaultsToHybridAstar)
{
    SimEngine engine;
    auto model = fleetsim::domain::vehicle::createVehicleModel("bicycle", 0.5, 1.0, 0.9, 0.6);
    auto vehicle = std::make_unique<Vehicle>("car_0", 1.2, Pose{2.0, 2.0, 0.0}, std::move(model));
    vehicle->setModelKind("bicycle");
    vehicle->setWheelbaseM(0.9);
    vehicle->setMaxSteeringRad(0.6);
    engine.setPlannerKind("auto");
    EXPECT_EQ(engine.resolvedPlannerKind(*vehicle), "hybrid_astar");
    engine.addVehicle(std::move(vehicle));
}

TEST(PlannerSwitchIntegrationTest, DiffDriveAutoDefaultsToAstar)
{
    SimEngine engine;
    auto vehicle = std::make_unique<Vehicle>("agv_0", 1.0, Pose{1.0, 1.0, 0.0});
    vehicle->setModelKind("diff_drive");
    engine.setPlannerKind("auto");
    EXPECT_EQ(engine.resolvedPlannerKind(*vehicle), "astar");
}

TEST(PlannerSwitchIntegrationTest, ExplicitAstarOverridesBicycleDefault)
{
    SimEngine engine;
    auto model = fleetsim::domain::vehicle::createVehicleModel("bicycle", 0.5, 1.0, 0.9, 0.6);
    auto vehicle = std::make_unique<Vehicle>("car_0", 1.2, Pose{2.0, 2.0, 0.0}, std::move(model));
    vehicle->setModelKind("bicycle");
    engine.setPlannerKind("astar");
    EXPECT_EQ(engine.resolvedPlannerKind(*vehicle), "astar");
}

TEST(PlannerSwitchIntegrationTest, ScenarioLoadsPlannerFieldAndPlansHybridNarrowTurn)
{
    const auto dir = findScenarioDir("hybrid_narrow_turn");
    if (dir.empty()) {
        GTEST_SKIP() << "hybrid_narrow_turn assets not found from cwd";
    }

    const auto data = ScenarioLoader::loadFromDirectory(dir.string(), 0.25);
    EXPECT_EQ(data.simulation.planner, "hybrid_astar");
    ASSERT_FALSE(data.vehicles.empty());
    EXPECT_EQ(data.vehicles.front().model, "bicycle");

    SimEngine engine;
    engine.setMap(data.map);
    engine.setPlannerKind(data.simulation.planner);
    engine.setTrackerKind(data.simulation.tracker);

    auto model = fleetsim::domain::vehicle::createVehicleModel(
        "bicycle", 0.5, 1.0,
        data.vehicles.front().wheelbase_m,
        data.vehicles.front().max_steering_rad);
    auto vehicle = std::make_unique<Vehicle>(
        data.vehicles.front().id,
        data.vehicles.front().length_m,
        data.vehicles.front().initial_pose,
        std::move(model));
    vehicle->setModelKind("bicycle");
    vehicle->setWheelbaseM(data.vehicles.front().wheelbase_m);
    vehicle->setMaxSteeringRad(data.vehicles.front().max_steering_rad);
    const Pose start = vehicle->pose();
    engine.addVehicle(std::move(vehicle));
    engine.setSelectedVehicle(data.vehicles.front().id);

    ASSERT_NE(engine.vehicle(), nullptr);
    ASSERT_EQ(engine.resolvedPlannerKind(*engine.vehicle()), "hybrid_astar");

    engine.setGoal(Pose{6.5, 7.5, 1.57});
    ASSERT_TRUE(engine.planPath());
    const auto hybrid_via_engine = engine.referencePath();
    ASSERT_GE(hybrid_via_engine.size(), 3U);

    const double dir0 = std::atan2(
        hybrid_via_engine.waypoints()[1].y - hybrid_via_engine.waypoints()[0].y,
        hybrid_via_engine.waypoints()[1].x - hybrid_via_engine.waypoints()[0].x);
    EXPECT_NEAR(dir0, start.theta, 0.5);

    // Same map/goal with forced A*+DP must not produce an identical waypoint list
    // (guards against ignoring hybrid_astar and always running A*).
    engine.setPlannerKind("astar");
    ASSERT_TRUE(engine.planPath());
    const auto astar_via_engine = engine.referencePath();
    ASSERT_FALSE(astar_via_engine.empty());
    EXPECT_FALSE(pathsNearlyIdentical(hybrid_via_engine, astar_via_engine))
        << "Identical Hybrid vs A*+DP paths imply SimEngine Hybrid branch was skipped";
}

TEST(PlannerSwitchIntegrationTest, DiffDriveAutoPlanPathStillSucceeds)
{
    const auto dir = findScenarioDir("demo");
    if (dir.empty()) {
        GTEST_SKIP() << "demo assets not found from cwd";
    }

    const auto data = ScenarioLoader::loadFromDirectory(dir.string());
    SimEngine engine;
    engine.setMap(data.map);
    engine.setPlannerKind("auto");

    auto vehicle = std::make_unique<Vehicle>(
        data.vehicles.front().id,
        data.vehicles.front().length_m,
        data.vehicles.front().initial_pose);
    vehicle->setModelKind("diff_drive");
    EXPECT_EQ(engine.resolvedPlannerKind(*vehicle), "astar");
    engine.addVehicle(std::move(vehicle));
    engine.setSelectedVehicle(data.vehicles.front().id);
    engine.setGoal(Pose{18.0, 12.0, 0.0});
    ASSERT_TRUE(engine.planPath());
    EXPECT_FALSE(engine.referencePath().empty());
}

TEST(PlannerSwitchIntegrationTest, HybridHeadingFidelityBeatsGridAStarOnNarrowTurn)
{
    const auto dir = findScenarioDir("hybrid_narrow_turn");
    if (dir.empty()) {
        GTEST_SKIP() << "hybrid_narrow_turn assets not found from cwd";
    }

    const auto data = ScenarioLoader::loadFromDirectory(dir.string(), 0.25);
    const Pose start = data.vehicles.front().initial_pose;
    const Pose goal{6.5, 7.5, 1.57};

    HybridAStarPlanner hybrid(
        data.vehicles.front().wheelbase_m,
        data.vehicles.front().max_steering_rad,
        0.2,
        3);
    AStarPlanner astar;

    const auto hybrid_path = hybrid.plan(data.map, start, goal);
    const auto astar_path = astar.plan(data.map, start, goal);

    ASSERT_FALSE(hybrid_path.empty());
    ASSERT_FALSE(astar_path.empty());
    ASSERT_GE(hybrid_path.size(), 3U);
    ASSERT_GE(astar_path.size(), 2U);

    const double hybrid_dir0 = std::atan2(
        hybrid_path.waypoints()[1].y - hybrid_path.waypoints()[0].y,
        hybrid_path.waypoints()[1].x - hybrid_path.waypoints()[0].x);
    const double astar_dir0 = std::atan2(
        astar_path.waypoints()[1].y - astar_path.waypoints()[0].y,
        astar_path.waypoints()[1].x - astar_path.waypoints()[0].x);

    EXPECT_NEAR(hybrid_dir0, start.theta, 0.5);
    EXPECT_LT(std::abs(hybrid_dir0 - start.theta), std::abs(astar_dir0 - start.theta) + 0.05)
        << "Hybrid first-segment heading should match start θ at least as well as grid A*";
    EXPECT_FALSE(pathsNearlyIdentical(hybrid_path, astar_path));
}
