#include "domain/collision/PriorityPathCoordinator.h"
#include "domain/SimEngine.h"
#include "domain/map/OccupancyGrid.h"
#include "domain/vehicle/Vehicle.h"

#include <gtest/gtest.h>

#include <cmath>
#include <memory>

using fleetsim::core::Path;
using fleetsim::core::Pose;
using fleetsim::core::Waypoint;
using fleetsim::domain::SimEngine;
using fleetsim::domain::collision::PriorityPathCoordinator;
using fleetsim::domain::map::OccupancyGrid;
using fleetsim::domain::vehicle::FleetManager;
using fleetsim::domain::vehicle::Vehicle;
using fleetsim::domain::vehicle::VehicleAgent;

namespace {

OccupancyGrid makeOpenGrid(int rows = 40, int cols = 40, double res = 0.25)
{
    OccupancyGrid grid(rows, cols, res, cols * res, rows * res);
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            grid.setOccupied(r, c, false);
        }
    }
    return grid;
}

bool pathUsesCellNear(const Path& path, const OccupancyGrid& grid, double x, double y, double tol)
{
    for (const Waypoint& w : path.waypoints()) {
        if (std::hypot(w.x - x, w.y - y) <= tol) {
            return true;
        }
        const auto cell = grid.worldToCell(w.x, w.y);
        const auto target = grid.worldToCell(x, y);
        if (std::abs(cell.row - target.row) <= 1 && std::abs(cell.col - target.col) <= 1) {
            return true;
        }
    }
    return false;
}

}  // namespace

TEST(PriorityCoordinatorTest, OrdersByTaskPriorityThenId)
{
    FleetManager fleet;
    fleet.addVehicle(std::make_unique<Vehicle>("b_low", 1.0, Pose{0, 0, 0}));
    fleet.addVehicle(std::make_unique<Vehicle>("a_high", 1.0, Pose{1, 0, 0}));
    fleet.addVehicle(std::make_unique<Vehicle>("c_mid", 1.0, Pose{2, 0, 0}));
    fleet.agent(0).task_priority = 1;
    fleet.agent(1).task_priority = 5;
    fleet.agent(2).task_priority = 5;

    const auto order = PriorityPathCoordinator::orderedAgentIndices(fleet);
    ASSERT_EQ(order.size(), 3U);
    // priority 5: a_high before c_mid (id ascending); then b_low
    EXPECT_EQ(fleet.agent(order[0]).vehicle->id(), "a_high");
    EXPECT_EQ(fleet.agent(order[1]).vehicle->id(), "c_mid");
    EXPECT_EQ(fleet.agent(order[2]).vehicle->id(), "b_low");
}

TEST(PriorityCoordinatorTest, PaintPathOccupiesCells)
{
    OccupancyGrid grid = makeOpenGrid();
    Path path(std::vector<Waypoint>{{1.0, 1.0}, {2.0, 1.0}, {3.0, 1.0}});
    PriorityPathCoordinator::paintPathOccupied(grid, path, 0);
    const auto cell = grid.worldToCell(2.0, 1.0);
    EXPECT_TRUE(grid.isOccupied(cell.row, cell.col));
}

TEST(PriorityCoordinatorTest, LowerPriorityDetoursAroundHigherPriorityCorridor)
{
    // Crossing goals: high priority claims center corridor first; low priority must detour.
    SimEngine engine;
    engine.setMap(makeOpenGrid());
    engine.setPlannerKind("astar");
    engine.setCoordinationKind("priority");

    engine.addVehicle(std::make_unique<Vehicle>("high", 1.0, Pose{1.0, 5.0, 0.0}));
    engine.addVehicle(std::make_unique<Vehicle>("low", 1.0, Pose{5.0, 1.0, 1.57}));

    VehicleAgent* high = engine.fleet().findAgent("high");
    VehicleAgent* low = engine.fleet().findAgent("low");
    ASSERT_NE(high, nullptr);
    ASSERT_NE(low, nullptr);
    high->task_priority = 10;
    low->task_priority = 1;
    high->goal = Pose{9.0, 5.0, 0.0};
    low->goal = Pose{5.0, 9.0, 1.57};
    high->needs_replan = true;
    low->needs_replan = true;

    engine.tick(0.05);

    ASSERT_FALSE(high->reference_path.empty());
    ASSERT_FALSE(low->reference_path.empty());

    // High path should stay near y=5 corridor.
    EXPECT_TRUE(pathUsesCellNear(high->reference_path, engine.map(), 5.0, 5.0, 0.6));

    // Low path should not share the same center cell as high's straight corridor
    // (inflate=1 blocks neighborhood). Crossing at (5,5) should be avoided by low.
    const bool low_hits_center =
        pathUsesCellNear(low->reference_path, engine.map(), 5.0, 5.0, 0.35);
    EXPECT_FALSE(low_hits_center)
        << "Lower priority must detour; identical corridor ownership is not Priority coordination";
}

TEST(PriorityCoordinatorTest, TimeWindowStillScalesWhenReservationsConflict)
{
    // Priority paints space; TimeWindow remains for runtime scaling (not deleted).
    SimEngine engine;
    engine.setMap(makeOpenGrid());
    engine.setCoordinationKind("priority");
    engine.addVehicle(std::make_unique<Vehicle>("a", 1.0, Pose{1.0, 2.0, 0.0}));
    engine.addVehicle(std::make_unique<Vehicle>("b", 1.0, Pose{1.0, 2.5, 0.0}));

    auto* a = engine.fleet().findAgent("a");
    auto* b = engine.fleet().findAgent("b");
    a->task_priority = 2;
    b->task_priority = 1;
    a->goal = Pose{8.0, 2.0, 0.0};
    b->goal = Pose{8.0, 2.5, 0.0};
    a->needs_replan = true;
    b->needs_replan = true;
    engine.tick(0.05);

    EXPECT_GT(engine.collision().avoidance().table().reservationCount(), 0U);
    // adjustVelocity path still invoked each tick
    engine.tick(0.05);
    EXPECT_TRUE(a->speed_scale > 0.0);
    EXPECT_TRUE(b->speed_scale > 0.0);
}
