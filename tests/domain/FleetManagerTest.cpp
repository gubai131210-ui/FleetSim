#include "domain/vehicle/FleetManager.h"
#include "domain/vehicle/Vehicle.h"

#include <gtest/gtest.h>

#include <memory>

using fleetsim::core::Pose;
using fleetsim::core::Task;
using fleetsim::domain::vehicle::AgentPhase;
using fleetsim::domain::vehicle::FleetManager;
using fleetsim::domain::vehicle::Vehicle;

TEST(FleetManagerTest, StartTaskSetsPickupPhase)
{
    FleetManager fleet;
    fleet.addVehicle(std::make_unique<Vehicle>("agv_0", 1.0, core::Pose{0.0, 0.0, 0.0}));

    Task task;
    task.id = "task_0";
    task.pickup = {2.0, 2.0, 0.0};
    task.dropoff = {8.0, 8.0, 0.0};
    fleet.startTask("agv_0", task);

    const auto* agent = fleet.findAgent("agv_0");
    ASSERT_NE(agent, nullptr);
    EXPECT_EQ(agent->phase, AgentPhase::ToPickup);
    EXPECT_TRUE(agent->needs_replan);
    EXPECT_DOUBLE_EQ(agent->goal.x, 2.0);
}

TEST(FleetManagerTest, IdleVehicleStatesOnlyReturnsIdleAgents)
{
    FleetManager fleet;
    fleet.addVehicle(std::make_unique<Vehicle>("agv_0", 1.0, core::Pose{0.0, 0.0, 0.0}));
    EXPECT_EQ(fleet.idleVehicleStates().size(), 1U);

    Task task;
    task.id = "task_0";
    task.pickup = {1.0, 1.0, 0.0};
    task.dropoff = {2.0, 2.0, 0.0};
    fleet.startTask("agv_0", task);
    EXPECT_TRUE(fleet.idleVehicleStates().empty());
}
