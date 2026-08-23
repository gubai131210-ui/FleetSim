#include "app/SimController.h"
#include "domain/scenario/ScenarioLoader.h"

#include <gtest/gtest.h>

#include <filesystem>

using fleetsim::app::SimController;
using fleetsim::domain::scenario::ScenarioLoader;

namespace {

std::string btNavigationDemoDir()
{
    const std::filesystem::path source_root =
        std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
    return (source_root / "assets" / "scenarios" / "bt_navigation_demo").string();
}

}  // namespace

TEST(BtNavigationDemoScenarioTest, LoadsBehaviorModeAndTreeFromScenarioDirectory)
{
    const auto scenario = ScenarioLoader::loadFromDirectory(btNavigationDemoDir());
    EXPECT_EQ(scenario.simulation.behavior_mode, "bt");
    EXPECT_EQ(scenario.simulation.behavior_tree_path, "navigate_replan_recovery.json");
    EXPECT_NEAR(scenario.simulation.replan_hz, 1.0, 1e-9);
    EXPECT_EQ(scenario.simulation.recovery_wait_ticks, 15);

    SimController controller;
    ASSERT_TRUE(controller.loadScenarioData(scenario));
    EXPECT_EQ(controller.engine().behaviorMode(), "bt");
    ASSERT_NE(controller.engine().btNavigator(), nullptr);
    EXPECT_TRUE(controller.engine().btNavigator()->hasTree());
}

TEST(BtNavigationDemoScenarioTest, PlansAndAdvancesUnderBtMode)
{
    SimController controller;
    ASSERT_TRUE(controller.loadScenario(btNavigationDemoDir()));
    controller.engine().setGoal({18.0, 12.0, 0.0});

    for (int i = 0; i < 120; ++i) {
        controller.stepOnce();
        if (!controller.engine().referencePath().empty()) {
            break;
        }
    }
    EXPECT_FALSE(controller.engine().referencePath().empty());

    const double start_x = controller.engine().vehicle()->pose().x;
    for (int i = 0; i < 200; ++i) {
        controller.stepOnce();
    }
    EXPECT_GT(controller.engine().vehicle()->pose().x, start_x);
}

TEST(BtNavigationDemoScenarioTest, RecoveryReplansAfterBlockedGoal)
{
    SimController controller;
    ASSERT_TRUE(controller.loadScenario(btNavigationDemoDir()));
    controller.engine().setGoal({9.0, 7.0, 0.0});

    for (int i = 0; i < 60; ++i) {
        controller.stepOnce();
    }
    EXPECT_TRUE(controller.engine().referencePath().empty());

    controller.engine().setGoal({18.0, 12.0, 0.0});
    for (int i = 0; i < 160; ++i) {
        controller.stepOnce();
        if (!controller.engine().referencePath().empty()) {
            break;
        }
    }
    EXPECT_FALSE(controller.engine().referencePath().empty());
}
