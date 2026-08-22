#include "domain/scenario/ScenarioLoader.h"

#include <gtest/gtest.h>

#include <filesystem>

using fleetsim::domain::scenario::ScenarioLoader;

namespace {

std::string demoScenarioDir()
{
    const std::filesystem::path source_dir = std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
    return (source_dir / "assets" / "scenarios" / "demo").string();
}

}  // namespace

TEST(ScenarioLoaderTest, LoadsDemoScenario)
{
    const auto scenario = ScenarioLoader::loadFromDirectory(demoScenarioDir());
    EXPECT_EQ(scenario.vehicles.size(), 1U);
    EXPECT_DOUBLE_EQ(scenario.simulation.dt_s, 0.05);
    EXPECT_GT(scenario.map.rows(), 0);
    EXPECT_EQ(scenario.vehicles.front().id, "agv_0");
}
