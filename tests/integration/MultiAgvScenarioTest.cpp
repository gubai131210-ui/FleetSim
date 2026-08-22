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

std::string multiAgvScenarioDir()
{
    const std::filesystem::path source_dir =
        std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
    return (source_dir / "assets" / "scenarios" / "multi_agv").string();
}

}  // namespace

TEST(MultiAgvScenarioTest, GreedyAssignmentAndTaskProgress)
{
    const auto scenario = ScenarioLoader::loadFromDirectory(multiAgvScenarioDir());

    SimEngine engine;
    engine.setMap(scenario.map);
    engine.loadTasks(scenario.tasks);

    for (const auto& vehicle_config : scenario.vehicles) {
        engine.addVehicle(std::make_unique<Vehicle>(
            vehicle_config.id,
            vehicle_config.length_m,
            vehicle_config.initial_pose));
    }

    for (int i = 0; i < 2000; ++i) {
        engine.tick(0.05);
    }

    EXPECT_GE(engine.scheduling().tasks().assignedCount() + engine.scheduling().tasks().doneCount(), 1);
    EXPECT_GE(engine.scheduling().tasks().doneCount(), 1);
}
