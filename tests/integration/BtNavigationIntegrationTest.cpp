#include "domain/SimEngine.h"
#include "domain/scenario/ScenarioLoader.h"
#include "domain/vehicle/Vehicle.h"
#include "app/SimController.h"

#include <gtest/gtest.h>

#include <filesystem>

using fleetsim::domain::SimEngine;
using fleetsim::domain::behavior::NodeStatus;
using fleetsim::domain::scenario::ScenarioLoader;
using fleetsim::domain::vehicle::Vehicle;

namespace {

std::filesystem::path repoRoot()
{
    return std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
}

std::string demoScenarioDir()
{
    return (repoRoot() / "assets" / "scenarios" / "demo").string();
}

std::string behaviorTreeAssetPath()
{
    return (repoRoot() / "assets" / "behavior_trees" / "navigate_replan_recovery.json").string();
}

void loadDemoVehicle(SimEngine& engine)
{
    const auto scenario = ScenarioLoader::loadFromDirectory(demoScenarioDir());
    engine.setMap(scenario.map);
    engine.clearFleet();
    const auto& vehicle_config = scenario.vehicles.front();
    engine.setVehicle(std::make_unique<Vehicle>(
        vehicle_config.id,
        vehicle_config.length_m,
        vehicle_config.initial_pose));
}

}  // namespace

TEST(BtNavigationIntegrationTest, BtModePlansPathViaBehaviorTree)
{
    SimEngine engine;
    loadDemoVehicle(engine);
    engine.setBehaviorMode("bt");
    engine.setReplanHz(1.0);
    engine.setRecoveryWaitTicks(5);
    ASSERT_TRUE(engine.loadBehaviorTree(behaviorTreeAssetPath()));
    engine.setGoal({18.0, 12.0, 0.0});

    for (int i = 0; i < 80; ++i) {
        engine.tick(0.05);
        if (!engine.referencePath().empty()) {
            break;
        }
    }

    EXPECT_FALSE(engine.referencePath().empty());
    EXPECT_NE(engine.lastBtTickResult().status, NodeStatus::Failure);
}

TEST(BtNavigationIntegrationTest, LegacyModeKeepsNeedsReplanPlanning)
{
    SimEngine engine;
    loadDemoVehicle(engine);
    EXPECT_EQ(engine.behaviorMode(), "legacy");
    engine.setGoal({18.0, 12.0, 0.0});
    engine.tick(0.05);
    EXPECT_FALSE(engine.referencePath().empty());
}

TEST(BtNavigationIntegrationTest, RecoveryAllowsReplanAfterBlockedGoal)
{
    SimEngine engine;
    loadDemoVehicle(engine);
    engine.setBehaviorMode("bt");
    engine.setRecoveryWaitTicks(3);
    ASSERT_TRUE(engine.loadBehaviorTree(behaviorTreeAssetPath()));

    engine.setGoal({9.0, 7.0, 0.0});
    for (int i = 0; i < 40; ++i) {
        engine.tick(0.05);
    }
    EXPECT_TRUE(engine.referencePath().empty());

    engine.setGoal({18.0, 12.0, 0.0});
    for (int i = 0; i < 120; ++i) {
        engine.tick(0.05);
        if (!engine.referencePath().empty()) {
            break;
        }
    }
    EXPECT_FALSE(engine.referencePath().empty());
}

TEST(BtNavigationIntegrationTest, SimControllerAppliesBehaviorModeFromScenario)
{
    const auto scenario = ScenarioLoader::loadFromDirectory(demoScenarioDir());
    fleetsim::app::SimController controller;
    auto scenario_mut = scenario;
    scenario_mut.simulation.behavior_mode = "bt";
    scenario_mut.simulation.behavior_tree_path = "navigate_replan_recovery.json";
    scenario_mut.simulation.recovery_wait_ticks = 5;
    ASSERT_TRUE(controller.loadScenarioData(std::move(scenario_mut)));
    EXPECT_EQ(controller.engine().behaviorMode(), "bt");
    ASSERT_NE(controller.engine().btNavigator(), nullptr);
    EXPECT_TRUE(controller.engine().btNavigator()->hasTree());
}
