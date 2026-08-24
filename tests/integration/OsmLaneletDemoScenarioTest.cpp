#include "app/SimController.h"
#include "domain/SimEngine.h"
#include "domain/scenario/ScenarioLoader.h"

#include <gtest/gtest.h>

#include <filesystem>

using fleetsim::app::SimController;
using fleetsim::domain::SimEngine;
using fleetsim::domain::scenario::ScenarioLoader;

namespace {

std::filesystem::path repoRoot()
{
    return std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
}

std::string osmLaneletDemoDir()
{
    return (repoRoot() / "assets" / "scenarios" / "osm_lanelet_demo").string();
}

}  // namespace

TEST(OsmLaneletDemoScenarioTest, LoadsOsmMapSourceFields)
{
    const auto scenario = ScenarioLoader::loadFromDirectory(osmLaneletDemoDir());
    EXPECT_EQ(scenario.simulation.map_source, "osm");
    EXPECT_FALSE(scenario.simulation.osm_path.empty());
    EXPECT_EQ(scenario.simulation.routing_mode, "hybrid");
}

TEST(OsmLaneletDemoScenarioTest, SimControllerImportsNonEmptyLaneGraph)
{
    SimController controller;
    ASSERT_TRUE(controller.loadScenario(osmLaneletDemoDir()));
    ASSERT_NE(controller.scenario(), nullptr);

    EXPECT_GE(controller.scenario()->lanes.nodes.size(), 2U);
    EXPECT_GE(controller.scenario()->lanes.edges.size(), 1U);
}

TEST(OsmLaneletDemoScenarioTest, HybridAndLaneGraphPlanningAfterOsmImport)
{
    SimEngine engine;
    const auto scenario = ScenarioLoader::loadFromDirectory(osmLaneletDemoDir());
    engine.setMap(scenario.map);
    engine.setRoutingMode("hybrid");
    engine.setLaneSnapRadiusM(scenario.simulation.lane_snap_radius_m);

    SimController controller;
    ASSERT_TRUE(controller.loadScenarioData(scenario));
    controller.engine().setRoutingMode("hybrid");
    controller.setGoal(7.5, 0.0, 0.0);
    ASSERT_TRUE(controller.planPath());
    EXPECT_FALSE(controller.engine().referencePath().empty());

    controller.engine().setRoutingMode("lane_graph");
    controller.setGoal(7.5, 0.0, 0.0);
    ASSERT_TRUE(controller.planPath());
    EXPECT_FALSE(controller.engine().referencePath().empty());
}
