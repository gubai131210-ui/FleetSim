#include "domain/SimEngine.h"
#include "domain/map/LaneTypes.h"
#include "domain/map/OccupancyGrid.h"
#include "domain/vehicle/Vehicle.h"

#include <cmath>
#include <gtest/gtest.h>

using fleetsim::core::Pose;
using fleetsim::domain::SimEngine;
using fleetsim::domain::map::LaneEdge;
using fleetsim::domain::map::LaneMapData;
using fleetsim::domain::map::LaneNode;
using fleetsim::domain::map::OccupancyGrid;
using fleetsim::domain::vehicle::Vehicle;

namespace {

OccupancyGrid makeOpenGrid()
{
    return OccupancyGrid(20, 20, 0.5, 10.0, 10.0);
}

LaneMapData makeCorridorLanes()
{
    LaneMapData lanes;
    lanes.nodes = {
        {"n0", 1.0, 2.0},
        {"n1", 4.0, 2.0},
        {"n2", 7.0, 2.0},
        {"n3", 9.0, 2.0},
    };
    lanes.edges = {
        {"n0", "n1", false},
        {"n1", "n2", false},
        {"n2", "n3", false},
    };
    return lanes;
}

void setupEngineWithCorridor(SimEngine& engine)
{
    engine.setMap(makeOpenGrid());
    engine.setLaneMap(makeCorridorLanes());
    engine.setLaneSnapRadiusM(2.0);
    engine.setPlannerKind("astar");

    auto vehicle = std::make_unique<Vehicle>("agv_0", 1.0, Pose{0.5, 0.5, 0.0});
    vehicle->setModelKind("diff_drive");
    engine.addVehicle(std::move(vehicle));
    engine.setSelectedVehicle("agv_0");
    engine.setGoal(Pose{9.5, 0.5, 0.0});
}

bool pathsDistinct(const fleetsim::core::Path& a, const fleetsim::core::Path& b)
{
    if (a.empty() || b.empty()) {
        return a.empty() != b.empty();
    }
    if (a.size() != b.size()) {
        return true;
    }
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (std::abs(a.waypoints()[i].x - b.waypoints()[i].x) > 0.15
            || std::abs(a.waypoints()[i].y - b.waypoints()[i].y) > 0.15) {
            return true;
        }
    }
    return false;
}

double meanWaypointY(const fleetsim::core::Path& path)
{
    if (path.empty()) {
        return 0.0;
    }
    double sum = 0.0;
    for (const auto& waypoint : path.waypoints()) {
        sum += waypoint.y;
    }
    return sum / static_cast<double>(path.size());
}

}  // namespace

TEST(FirstLastMileIntegrationTest, RoutingModeDefaultIsFreespace)
{
    SimEngine engine;
    EXPECT_EQ(engine.routingMode(), "freespace");
}

TEST(FirstLastMileIntegrationTest, LaneGraphModeFailsWithoutLanes)
{
    SimEngine engine;
    engine.setMap(makeOpenGrid());
    engine.setRoutingMode("lane_graph");

    auto vehicle = std::make_unique<Vehicle>("agv_0", 1.0, Pose{1.0, 1.0, 0.0});
    engine.addVehicle(std::move(vehicle));
    engine.setSelectedVehicle("agv_0");
    engine.setGoal(Pose{5.0, 5.0, 0.0});

    EXPECT_FALSE(engine.planPath());
}

TEST(FirstLastMileIntegrationTest, ThreeRoutingModesProduceDistinctPaths)
{
    SimEngine engine;
    setupEngineWithCorridor(engine);

    engine.setRoutingMode("freespace");
    ASSERT_TRUE(engine.planPath());
    const auto freespace_path = engine.referencePath();
    ASSERT_FALSE(freespace_path.empty());

    engine.setRoutingMode("lane_graph");
    ASSERT_TRUE(engine.planPath());
    const auto lane_path = engine.referencePath();
    ASSERT_FALSE(lane_path.empty());
    EXPECT_NEAR(meanWaypointY(lane_path), 2.0, 0.25);

    engine.setRoutingMode("hybrid");
    ASSERT_TRUE(engine.planPath());
    const auto hybrid_path = engine.referencePath();
    ASSERT_FALSE(hybrid_path.empty());

    EXPECT_TRUE(pathsDistinct(freespace_path, lane_path))
        << "lane_graph path should differ from freespace";
    EXPECT_TRUE(pathsDistinct(freespace_path, hybrid_path))
        << "hybrid path should differ from freespace";
    EXPECT_TRUE(pathsDistinct(lane_path, hybrid_path))
        << "hybrid path should differ from pure lane_graph";
}

TEST(FirstLastMileIntegrationTest, HybridPathIsLongerThanLaneGraphSegment)
{
    SimEngine engine;
    setupEngineWithCorridor(engine);

    engine.setRoutingMode("lane_graph");
    ASSERT_TRUE(engine.planPath());
    const double lane_length = engine.referencePath().length();

    engine.setRoutingMode("hybrid");
    ASSERT_TRUE(engine.planPath());
    const double hybrid_length = engine.referencePath().length();

    EXPECT_GT(hybrid_length, lane_length + 0.1)
        << "hybrid should include first/last mile beyond lane centerline";
}
