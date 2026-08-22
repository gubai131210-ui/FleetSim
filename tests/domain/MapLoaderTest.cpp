#include "domain/map/MapLoader.h"

#include <gtest/gtest.h>

#include <filesystem>

using fleetsim::domain::map::MapLoader;

namespace {

std::string demoMapPath()
{
    const std::filesystem::path source_dir = std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
    return (source_dir / "assets" / "scenarios" / "demo" / "map.json").string();
}

}  // namespace

TEST(MapLoaderTest, LoadsDemoMapDimensions)
{
    const auto metadata = MapLoader::loadMetadataFromFile(demoMapPath());
    EXPECT_DOUBLE_EQ(metadata.width_m, 20.0);
    EXPECT_DOUBLE_EQ(metadata.height_m, 15.0);
    EXPECT_DOUBLE_EQ(metadata.grid_resolution_m, 0.1);
}

TEST(MapLoaderTest, LoadsDemoObstacleCells)
{
    const auto grid = MapLoader::loadFromFile(demoMapPath());
    EXPECT_EQ(grid.rows(), 150);
    EXPECT_EQ(grid.cols(), 200);

    const auto obstacle_cell = grid.worldToCell(9.0, 7.0);
    EXPECT_TRUE(grid.isOccupied(obstacle_cell.row, obstacle_cell.col));

    const auto free_cell = grid.worldToCell(2.0, 2.0);
    EXPECT_FALSE(grid.isOccupied(free_cell.row, free_cell.col));
}
