#include "domain/SimEngine.h"
#include "domain/behavior/MultiBtNavigator.h"
#include "domain/scenario/ScenarioLoader.h"
#include "domain/vehicle/Vehicle.h"

#include <gtest/gtest.h>

#include <filesystem>

using fleetsim::domain::SimEngine;
using fleetsim::domain::behavior::MultiBtNavigator;
using fleetsim::domain::behavior::NodeStatus;
using fleetsim::domain::scenario::ScenarioLoader;
using fleetsim::domain::vehicle::Vehicle;

namespace BbKey = fleetsim::domain::behavior::BbKey;

namespace {

std::filesystem::path repoRoot()
{
    return std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
}

std::string behaviorTreeAssetPath()
{
    return (repoRoot() / "assets" / "behavior_trees" / "navigate_replan_recovery.json").string();
}

void loadDemoMap(SimEngine& engine)
{
    const auto scenario =
        ScenarioLoader::loadFromDirectory((repoRoot() / "assets" / "scenarios" / "demo").string());
    engine.setMap(scenario.map);
}

}  // namespace

TEST(MultiBtNavigationTest, SeparateBlackboardsDoNotOverwriteAgentId)
{
    MultiBtNavigator multi;

    auto& bb0 = multi.blackboardFor("agv_0");
    auto& bb1 = multi.blackboardFor("agv_1");

    EXPECT_EQ(bb0.getString(BbKey::kAgentId).value_or(""), "agv_0");
    EXPECT_EQ(bb1.getString(BbKey::kAgentId).value_or(""), "agv_1");
}

TEST(MultiBtNavigationTest, DistinctNavigatorInstances)
{
    MultiBtNavigator multi;
    EXPECT_NE(&multi.navigatorFor("a"), &multi.navigatorFor("b"));
    EXPECT_NE(&multi.blackboardFor("a"), &multi.blackboardFor("b"));
    EXPECT_EQ(multi.agentCount(), 2U);
}

TEST(MultiBtNavigationTest, TickIsolationDoesNotCrossContaminateBlackboards)
{
    MultiBtNavigator multi;
    ASSERT_TRUE(multi.loadTreeForAgent("agv_0", behaviorTreeAssetPath(), "json"));
    ASSERT_TRUE(multi.loadTreeForAgent("agv_1", behaviorTreeAssetPath(), "json"));

    multi.blackboardFor("agv_0").setBool(BbKey::kPathValid, true);
    multi.blackboardFor("agv_0").setInt(BbKey::kRecoveryCount, 7);
    multi.blackboardFor("agv_1").setBool(BbKey::kPathValid, false);
    multi.blackboardFor("agv_1").setInt(BbKey::kRecoveryCount, 3);

    EXPECT_EQ(multi.blackboardFor("agv_0").getString(BbKey::kAgentId).value_or(""), "agv_0");
    EXPECT_EQ(multi.blackboardFor("agv_1").getString(BbKey::kAgentId).value_or(""), "agv_1");
    EXPECT_TRUE(multi.blackboardFor("agv_0").getBool(BbKey::kPathValid).value_or(false));
    EXPECT_FALSE(multi.blackboardFor("agv_1").getBool(BbKey::kPathValid).value_or(true));
    EXPECT_EQ(multi.blackboardFor("agv_0").getInt(BbKey::kRecoveryCount).value_or(0), 7);
    EXPECT_EQ(multi.blackboardFor("agv_1").getInt(BbKey::kRecoveryCount).value_or(0), 3);
}

TEST(MultiBtNavigationTest, SimEngineTicksAllAgentsWithSeparateBlackboards)
{
    SimEngine engine;
    loadDemoMap(engine);
    engine.setBehaviorMode("bt");
    engine.setReplanHz(1.0);
    engine.setRecoveryWaitTicks(5);

    engine.addVehicle(std::make_unique<Vehicle>("agv_0", 1.0, fleetsim::core::Pose{2.0, 2.0, 0.0}));
    engine.addVehicle(std::make_unique<Vehicle>("agv_1", 1.0, fleetsim::core::Pose{4.0, 2.0, 0.0}));
    ASSERT_TRUE(engine.loadBehaviorTree(behaviorTreeAssetPath()));

    engine.setSelectedVehicle("agv_0");
    engine.setGoal({18.0, 12.0, 0.0});
    engine.tick(0.05);

    const auto& bb0 = engine.multiBtNavigator().blackboardFor("agv_0");
    const auto& bb1 = engine.multiBtNavigator().blackboardFor("agv_1");
    EXPECT_EQ(bb0.getString(BbKey::kAgentId).value_or(""), "agv_0");
    EXPECT_EQ(bb1.getString(BbKey::kAgentId).value_or(""), "agv_1");
    EXPECT_NE(engine.lastBtTickResult().status, NodeStatus::Failure);
    ASSERT_NE(engine.btNavigator(), nullptr);
    EXPECT_TRUE(engine.btNavigator()->hasTree());
}
