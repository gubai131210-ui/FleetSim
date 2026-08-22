#include "domain/map/MapSerializer.h"

#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

using fleetsim::domain::map::MapDocument;
using fleetsim::domain::map::MapSerializer;
using fleetsim::domain::map::Obstacle;
using fleetsim::domain::map::ObstacleType;
using fleetsim::domain::map::PolygonObstacle;
using fleetsim::domain::map::RectObstacle;

TEST(MapSerializerTest, RoundTripRectObstacle)
{
    MapDocument document;
    document.width_m = 10.0;
    document.height_m = 8.0;
    document.grid_resolution_m = 0.1;

    Obstacle obstacle;
    obstacle.type = ObstacleType::Rect;
    obstacle.rect = {1.0, 2.0, 3.0, 4.0};
    document.obstacles.push_back(obstacle);

    const nlohmann::json json = MapSerializer::toJson(document);
    const MapDocument loaded = MapSerializer::fromJson(json);

    ASSERT_EQ(loaded.obstacles.size(), 1U);
    EXPECT_DOUBLE_EQ(loaded.obstacles.front().rect.width, 3.0);
}

TEST(MapSerializerTest, RejectsInvalidRect)
{
    EXPECT_FALSE(MapSerializer::isValidRect(RectObstacle{0.0, 0.0, 0.0, 1.0}));
    EXPECT_FALSE(MapSerializer::isValidRect(RectObstacle{0.0, 0.0, 1.0, -1.0}));
}

TEST(MapSerializerTest, RejectsInvalidPolygon)
{
    PolygonObstacle polygon;
    polygon.vertices = {{0.0, 0.0}, {1.0, 0.0}};
    EXPECT_FALSE(MapSerializer::isValidPolygon(polygon));
}

TEST(MapSerializerTest, EmptyObstaclesProducesFreeGrid)
{
    MapDocument document;
    document.width_m = 2.0;
    document.height_m = 2.0;
    document.grid_resolution_m = 1.0;

    const auto grid = MapSerializer::toOccupancyGrid(document);
    EXPECT_EQ(grid.rows(), 2);
    EXPECT_EQ(grid.cols(), 2);
    EXPECT_FALSE(grid.isOccupied(0, 0));
}

TEST(MapSerializerTest, OutOfBoundsRectIsSkippedDuringLoad)
{
    nlohmann::json json = {
        {"version", 1},
        {"width_m", 10.0},
        {"height_m", 10.0},
        {"grid_resolution_m", 0.1},
        {"obstacles",
         {{{"type", "rect"}, {"x", 0.0}, {"y", 0.0}, {"width", 0.0}, {"height", 1.0}}}},
    };

    const MapDocument loaded = MapSerializer::fromJson(json);
    EXPECT_TRUE(loaded.obstacles.empty());
}
