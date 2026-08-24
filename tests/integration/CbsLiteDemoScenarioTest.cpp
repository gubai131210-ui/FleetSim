#include "app/SimController.h"
#include "domain/SimEngine.h"
#include "domain/scenario/ScenarioLoader.h"
#include "domain/vehicle/Vehicle.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <memory>

using fleetsim::app::SimController;
using fleetsim::domain::SimEngine;
using fleetsim::domain::scenario::ScenarioLoader;
using fleetsim::domain::vehicle::Vehicle;

namespace {

std::filesystem::path repoRoot()
{
    return std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
}

std::string cbsLiteDemoDir()
{
    return (repoRoot() / "assets" / "scenarios" / "cbs_lite_demo").string();
}

void setupTwoVehicleEngine(SimEngine& engine, const fleetsim::domain::scenario::ScenarioData& scenario)
{
    engine.setMap(scenario.map);
    engine.setPlannerKind(scenario.simulation.planner);
    engine.setRoutingMode(scenario.simulation.routing_mode);
    engine.setCoordinationKind(scenario.simulation.coordination);

    fleetsim::domain::collision::CbsLiteConfig config;
    config.max_depth = scenario.simulation.cbs_max_depth;
    config.time_limit_ms = scenario.simulation.cbs_time_limit_ms;
    engine.setCbsLiteConfig(config);

    for (const auto& vehicle_config : scenario.vehicles) {
        engine.addVehicle(std::make_unique<Vehicle>(
            vehicle_config.id,
            vehicle_config.length_m,
            vehicle_config.initial_pose));
    }
}

}  // namespace

TEST(CbsLiteDemoScenarioTest, LoadsCbsLiteCoordinationFields)
{
    const auto scenario = ScenarioLoader::loadFromDirectory(cbsLiteDemoDir());
    EXPECT_EQ(scenario.simulation.coordination, "cbs_lite");
    EXPECT_GE(scenario.simulation.cbs_max_depth, 1);
    EXPECT_GE(scenario.vehicles.size(), 2U);
}

TEST(CbsLiteDemoScenarioTest, SimControllerAppliesCbsLiteConfig)
{
    SimController controller;
    ASSERT_TRUE(controller.loadScenario(cbsLiteDemoDir()));
    EXPECT_TRUE(controller.engine().usesCbsLiteCoordination());
    EXPECT_GE(controller.engine().cbsLiteConfig().max_depth, 1);
}

TEST(CbsLiteDemoScenarioTest, HeadOnGoalsProducePathsForBothAgents)
{
    const auto scenario = ScenarioLoader::loadFromDirectory(cbsLiteDemoDir());

    SimEngine engine;
    setupTwoVehicleEngine(engine, scenario);

    engine.setSelectedVehicle("agv_0");
    engine.setGoal({18.0, 5.0, 0.0});
    ASSERT_TRUE(engine.planPathFor("agv_0"));
    EXPECT_FALSE(engine.referencePath().empty());

    engine.setSelectedVehicle("agv_1");
    engine.setGoal({2.0, 5.0, 0.0});
    ASSERT_TRUE(engine.planPathFor("agv_1"));

    const auto* agent0 = engine.fleet().findAgent("agv_0");
    const auto* agent1 = engine.fleet().findAgent("agv_1");
    ASSERT_NE(agent0, nullptr);
    ASSERT_NE(agent1, nullptr);
    EXPECT_FALSE(agent0->reference_path.empty());
    EXPECT_FALSE(agent1->reference_path.empty());
}

TEST(CbsLiteDemoScenarioTest, SimulationTicksWithoutImmediateStall)
{
    SimController controller;
    ASSERT_TRUE(controller.loadScenario(cbsLiteDemoDir()));

    controller.engine().setSelectedVehicle("agv_0");
    controller.setGoal(18.0, 5.0, 0.0);
    ASSERT_TRUE(controller.planPath());

    controller.engine().setSelectedVehicle("agv_1");
    controller.setGoal(2.0, 5.0, 0.0);
    ASSERT_TRUE(controller.engine().planPathFor("agv_1"));

    for (int i = 0; i < 120; ++i) {
        controller.stepOnce();
    }

    const auto* agent0 = controller.engine().fleet().findAgent("agv_0");
    const auto* agent1 = controller.engine().fleet().findAgent("agv_1");
    ASSERT_NE(agent0, nullptr);
    ASSERT_NE(agent1, nullptr);
    EXPECT_GT(agent0->vehicle->pose().x, 1.5);
    EXPECT_LT(agent1->vehicle->pose().x, 18.5);
}
