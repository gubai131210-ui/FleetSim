#include "domain/collision/CollisionModule.h"
#include "domain/map/OccupancyGrid.h"
#include "domain/vehicle/FleetManager.h"
#include "domain/vehicle/Vehicle.h"

#include <gtest/gtest.h>

#include <memory>

using fleetsim::domain::collision::CollisionModule;
using fleetsim::domain::map::OccupancyGrid;
using fleetsim::domain::vehicle::FleetManager;
using fleetsim::domain::vehicle::Vehicle;

TEST(CollisionModule, CanInstantiateAndTick)
{
    CollisionModule module;
    FleetManager fleet;
    fleet.addVehicle(std::make_unique<Vehicle>("agv_0", 1.0, core::Pose{0.0, 0.0, 0.0}));
    OccupancyGrid grid(10, 10, 0.1, 1.0, 1.0);
    module.tick(0.05, fleet, 0.0, grid);
    EXPECT_GE(module.conflictCount(), 0);
}
