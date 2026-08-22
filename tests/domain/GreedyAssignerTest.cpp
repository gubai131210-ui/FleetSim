#include "domain/scheduling/GreedyAssigner.h"

#include <gtest/gtest.h>

using fleetsim::core::Task;
using fleetsim::core::VehicleState;
using fleetsim::domain::scheduling::GreedyAssigner;

TEST(GreedyAssignerTest, AssignsNearestVehicleToTask)
{
    GreedyAssigner assigner;

    Task task;
    task.id = "task_0";
    task.pickup = {10.0, 0.0, 0.0};

    VehicleState near;
    near.id = "agv_near";
    near.pose = {9.0, 0.0, 0.0};
    near.idle = true;

    VehicleState far;
    far.id = "agv_far";
    far.pose = {0.0, 0.0, 0.0};
    far.idle = true;

    const auto assignments = assigner.assign({task}, {near, far});
    ASSERT_EQ(assignments.size(), 1U);
    EXPECT_EQ(assignments.front().vehicle_id, "agv_near");
    EXPECT_EQ(assignments.front().task_id, "task_0");
}
