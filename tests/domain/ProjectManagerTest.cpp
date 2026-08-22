#include "app/ProjectManager.h"

#include <gtest/gtest.h>

#include <cstdlib>
#include <filesystem>

namespace {

std::filesystem::path tempScenarioDir()
{
    const auto dir = std::filesystem::temp_directory_path()
        / ("fleetsim_project_test_" + std::to_string(std::rand()));
    std::filesystem::create_directories(dir);
    return dir;
}

}  // namespace

TEST(ProjectManagerTest, SaveAndLoadRoundTrip)
{
    const auto dir = tempScenarioDir();
    fleetsim::app::ProjectManager manager;
    manager.newProject(dir.string());

    fleetsim::domain::map::Obstacle obstacle;
    obstacle.type = fleetsim::domain::map::ObstacleType::Rect;
    obstacle.rect = {5.0, 5.0, 2.0, 2.0};
    manager.mapDocument().obstacles.push_back(obstacle);

    ASSERT_TRUE(manager.save(dir.string()));

    fleetsim::app::ProjectManager reloaded;
    ASSERT_TRUE(reloaded.load(dir.string()));
    ASSERT_EQ(reloaded.mapDocument().obstacles.size(), 1U);
    EXPECT_DOUBLE_EQ(reloaded.mapDocument().obstacles.front().rect.x, 5.0);

    std::filesystem::remove_all(dir);
}

TEST(ProjectManagerTest, SaveWritesMapJson)
{
    const auto dir = tempScenarioDir();
    fleetsim::app::ProjectManager manager;
    manager.newProject(dir.string());
    ASSERT_TRUE(manager.save(dir.string()));
    EXPECT_TRUE(std::filesystem::exists(dir / "map.json"));
    EXPECT_TRUE(std::filesystem::exists(dir / "scenario.json"));
    std::filesystem::remove_all(dir);
}

TEST(ProjectManagerTest, LoadMissingProjectFails)
{
    fleetsim::app::ProjectManager manager;
    EXPECT_FALSE(manager.load("/path/that/does/not/exist"));
}
