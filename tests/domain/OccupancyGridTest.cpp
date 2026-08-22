#include "domain/map/OccupancyGrid.h"

#include <gtest/gtest.h>

using fleetsim::domain::map::OccupancyGrid;

TEST(OccupancyGrid, WorldToCellConversion)
{
    OccupancyGrid grid(150, 200, 0.1, 20.0, 15.0);
    const auto cell = grid.worldToCell(2.05, 3.05);
    EXPECT_EQ(cell.row, 30);
    EXPECT_EQ(cell.col, 20);
}

TEST(OccupancyGrid, InflationExpandsOccupiedCells)
{
    OccupancyGrid grid(50, 50, 0.1, 5.0, 5.0);
    grid.setOccupied(25, 25, true);

    EXPECT_FALSE(grid.isOccupied(25, 27));
    grid.inflate(0.25);
    EXPECT_TRUE(grid.isOccupied(25, 27));
}

TEST(OccupancyGrid, OutOfBoundsTreatedAsOccupied)
{
    OccupancyGrid grid(10, 10, 0.1, 1.0, 1.0);
    EXPECT_TRUE(grid.isOccupied(-1, 0));
    EXPECT_TRUE(grid.isOccupied(0, 100));
}
