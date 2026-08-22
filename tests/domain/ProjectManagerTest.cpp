#include "app/ProjectManager.h"

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <cstdlib>

namespace {

// Prefer ASCII cwd-relative dirs: system Temp often has non-ASCII user names on
 // Chinese Windows, and MinGW ofstream cannot open those paths.
std::filesystem::path tempScenarioDir()
{
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    const auto dir = std::filesystem::current_path()
        / "test_tmp"
        / ("fleetsim_project_" + std::to_string(stamp) + "_" + std::to_string(std::rand()));
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

    ASSERT_TRUE(manager.save(dir.string())) << "save failed for dir=" << dir.string();

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
    ASSERT_TRUE(manager.save(dir.string())) << "save failed for dir=" << dir.string();
    EXPECT_TRUE(std::filesystem::exists(dir / "map.json"));
    EXPECT_TRUE(std::filesystem::exists(dir / "scenario.json"));
    std::filesystem::remove_all(dir);
}

TEST(ProjectManagerTest, LoadMissingProjectFails)
{
    fleetsim::app::ProjectManager manager;
    EXPECT_FALSE(manager.load("/path/that/does/not/exist"));
}
