#include "domain/SimEngine.h"
#include "domain/scenario/ScenarioLoader.h"
#include "domain/vehicle/Vehicle.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <memory>

using fleetsim::domain::SimEngine;
using fleetsim::domain::scenario::ScenarioLoader;
using fleetsim::domain::vehicle::Vehicle;

namespace {

std::string demoScenarioDir()
{
    const std::filesystem::path source_dir = std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
    return (source_dir / "assets" / "scenarios" / "demo").string();
}

}  // namespace

TEST(DemoScenarioTest, LoadPlanAndAdvancePose)
{
    const auto scenario = ScenarioLoader::loadFromDirectory(demoScenarioDir());

    SimEngine engine;
    engine.setMap(scenario.map);

    const auto& vehicle_config = scenario.vehicles.front();
    engine.setVehicle(std::make_unique<Vehicle>(
        vehicle_config.id,
        vehicle_config.length_m,
        vehicle_config.initial_pose));

    engine.setGoal({18.0, 12.0, 0.0});
    ASSERT_TRUE(engine.planPath());
    ASSERT_FALSE(engine.referencePath().empty());

    const double start_x = engine.vehicle()->pose().x;
    for (int i = 0; i < 100; ++i) {
        engine.tick(0.05);
    }

    EXPECT_GT(engine.vehicle()->pose().x, start_x);
}
