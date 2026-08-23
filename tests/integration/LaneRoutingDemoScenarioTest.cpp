#include "domain/SimEngine.h"
#include "domain/scenario/ScenarioLoader.h"
#include "domain/vehicle/Vehicle.h"

#include <cmath>
#include <filesystem>
#include <gtest/gtest.h>
#include <memory>

using fleetsim::core::Pose;
using fleetsim::domain::SimEngine;
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
    };
    for (const auto& path : candidates) {
        if (std::filesystem::exists(path / "scenario.json")) {
            return path;
        }
    }
    return {};
}

bool pathsDistinct(const fleetsim::core::Path& a, const fleetsim::core::Path& b)
{
    if (a.empty() || b.empty()) {
        return a.empty() != b.empty();
    }
    if (a.size() != b.size()) {
        return true;
    }
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (std::abs(a.waypoints()[i].x - b.waypoints()[i].x) > 0.15
            || std::abs(a.waypoints()[i].y - b.waypoints()[i].y) > 0.15) {
            return true;
        }
    }
    return false;
}

void setupEngineFromScenario(SimEngine& engine, const fleetsim::domain::scenario::ScenarioData& scenario)
{
    engine.setMap(scenario.map);
    engine.setLaneMap(scenario.lanes);
    engine.setPlannerKind(scenario.simulation.planner.empty() ? "auto" : scenario.simulation.planner);
    engine.setRoutingMode(
        scenario.simulation.routing_mode.empty() ? "freespace" : scenario.simulation.routing_mode);
    engine.setLaneSnapRadiusM(scenario.simulation.lane_snap_radius_m);

    const auto& vehicle_config = scenario.vehicles.front();
    auto vehicle = std::make_unique<Vehicle>(
        vehicle_config.id,
        vehicle_config.length_m,
        vehicle_config.initial_pose);
    vehicle->setModelKind(vehicle_config.model);
    engine.addVehicle(std::move(vehicle));
    engine.setSelectedVehicle(vehicle_config.id);
}

}  // namespace

TEST(LaneRoutingDemoScenarioTest, LoadsForkLanesFromMapJson)
{
    const auto dir = findScenarioDir("lane_routing_demo");
    if (dir.empty()) {
        GTEST_SKIP() << "lane_routing_demo assets not found from cwd";
    }

    const auto scenario = ScenarioLoader::loadFromDirectory(dir.string());
    EXPECT_EQ(scenario.simulation.routing_mode, "hybrid");
    ASSERT_GE(scenario.lanes.nodes.size(), 4U);
    ASSERT_GE(scenario.lanes.edges.size(), 4U);

    bool has_fork = false;
    for (const auto& edge : scenario.lanes.edges) {
        if (edge.from == "n1" && (edge.to == "n2" || edge.to == "n3")) {
            has_fork = true;
        }
    }
    EXPECT_TRUE(has_fork);
}

TEST(LaneRoutingDemoScenarioTest, HybridLaneGraphAndFreespaceAllPlan)
{
    const auto dir = findScenarioDir("lane_routing_demo");
    if (dir.empty()) {
        GTEST_SKIP() << "lane_routing_demo assets not found from cwd";
    }

    const auto scenario = ScenarioLoader::loadFromDirectory(dir.string());
    const Pose goal{14.0, 0.5, 0.0};

    SimEngine hybrid_engine;
    setupEngineFromScenario(hybrid_engine, scenario);
    hybrid_engine.setRoutingMode("hybrid");
    hybrid_engine.setGoal(goal);
    ASSERT_TRUE(hybrid_engine.planPath());
    const auto hybrid_path = hybrid_engine.referencePath();
    ASSERT_FALSE(hybrid_path.empty());

    SimEngine lane_engine;
    setupEngineFromScenario(lane_engine, scenario);
    lane_engine.setRoutingMode("lane_graph");
    lane_engine.setGoal(goal);
    ASSERT_TRUE(lane_engine.planPath());
    const auto lane_path = lane_engine.referencePath();
    ASSERT_FALSE(lane_path.empty());

    SimEngine free_engine;
    setupEngineFromScenario(free_engine, scenario);
    free_engine.setRoutingMode("freespace");
    free_engine.setGoal(goal);
    ASSERT_TRUE(free_engine.planPath());
    const auto free_path = free_engine.referencePath();
    ASSERT_FALSE(free_path.empty());

    EXPECT_TRUE(pathsDistinct(hybrid_path, lane_path));
    EXPECT_TRUE(pathsDistinct(hybrid_path, free_path));
    EXPECT_TRUE(pathsDistinct(lane_path, free_path));
}

TEST(LaneRoutingDemoScenarioTest, DefaultFreespaceRegressionOnDemoScenario)
{
    const auto dir = findScenarioDir("demo");
    if (dir.empty()) {
        GTEST_SKIP() << "demo assets not found from cwd";
    }

    const auto scenario = ScenarioLoader::loadFromDirectory(dir.string());
    SimEngine engine;
    engine.setMap(scenario.map);
    engine.setLaneMap(scenario.lanes);
    engine.setRoutingMode("freespace");

    const auto& vehicle_config = scenario.vehicles.front();
    engine.setVehicle(std::make_unique<Vehicle>(
        vehicle_config.id,
        vehicle_config.length_m,
        vehicle_config.initial_pose));
    engine.setGoal({18.0, 12.0, 0.0});
    ASSERT_TRUE(engine.planPath());
    EXPECT_FALSE(engine.referencePath().empty());
}
