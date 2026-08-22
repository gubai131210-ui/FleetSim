#include "domain/scheduling/SchedulingModule.h"
#include "domain/vehicle/FleetManager.h"
#include "domain/vehicle/Vehicle.h"

#include <gtest/gtest.h>

#include <memory>

using fleetsim::domain::scheduling::SchedulingModule;
using fleetsim::domain::vehicle::FleetManager;
using fleetsim::domain::vehicle::Vehicle;

TEST(SchedulingModule, CanInstantiateAndTick)
{
    SchedulingModule module;
    FleetManager fleet;
    fleet.addVehicle(std::make_unique<Vehicle>("agv_0", 1.0, core::Pose{0.0, 0.0, 0.0}));
    module.tick(0.05, fleet);
    EXPECT_GE(module.pendingTaskCount(), 0);
}
