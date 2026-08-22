#include "domain/scheduling/GreedyAssigner.h"
#include "domain/scheduling/HungarianAssigner.h"

#include <cmath>
#include <gtest/gtest.h>
#include <vector>

using fleetsim::core::Task;
using fleetsim::core::VehicleState;
using fleetsim::domain::scheduling::GreedyAssigner;
using fleetsim::domain::scheduling::HungarianAssigner;
using fleetsim::domain::scheduling::TaskAssignment;

namespace {

double totalPickupCost(const std::vector<TaskAssignment>& assignments,
                       const std::vector<Task>& tasks,
                       const std::vector<VehicleState>& vehicles)
{
    double total = 0.0;
    for (const TaskAssignment& a : assignments) {
        const Task* task = nullptr;
        const VehicleState* vehicle = nullptr;
        for (const Task& t : tasks) {
            if (t.id == a.task_id) {
                task = &t;
                break;
            }
        }
        for (const VehicleState& v : vehicles) {
            if (v.id == a.vehicle_id) {
                vehicle = &v;
                break;
            }
        }
        if (task == nullptr || vehicle == nullptr) {
            continue;
        }
        const double dx = task->pickup.x - vehicle->pose.x;
        const double dy = task->pickup.y - vehicle->pose.y;
        total += std::sqrt(dx * dx + dy * dy);
    }
    return total;
}

}  // namespace

TEST(HungarianAssignerTest, SquareMatrixCostNotWorseThanGreedy)
{
    Task t0;
    t0.id = "t0";
    t0.pickup = {0.0, 0.0, 0.0};
    Task t1;
    t1.id = "t1";
    t1.pickup = {10.0, 0.0, 0.0};

    VehicleState v0;
    v0.id = "v0";
    v0.pose = {9.0, 0.0, 0.0};
    v0.idle = true;
    VehicleState v1;
    v1.id = "v1";
    v1.pose = {1.0, 0.0, 0.0};
    v1.idle = true;

    const std::vector<Task> tasks{t0, t1};
    const std::vector<VehicleState> vehicles{v0, v1};

    HungarianAssigner hungarian;
    GreedyAssigner greedy;
    const auto h = hungarian.assign(tasks, vehicles);
    const auto g = greedy.assign(tasks, vehicles);

    ASSERT_EQ(h.size(), 2u);
    EXPECT_LE(totalPickupCost(h, tasks, vehicles), totalPickupCost(g, tasks, vehicles) + 1e-9);
}

TEST(HungarianAssignerTest, RectangularMoreTasksThanVehicles)
{
    Task t0;
    t0.id = "t0";
    t0.pickup = {0.0, 0.0, 0.0};
    Task t1;
    t1.id = "t1";
    t1.pickup = {5.0, 0.0, 0.0};
    Task t2;
    t2.id = "t2";
    t2.pickup = {20.0, 0.0, 0.0};

    VehicleState v0;
    v0.id = "v0";
    v0.pose = {0.1, 0.0, 0.0};
    v0.idle = true;
    VehicleState v1;
    v1.id = "v1";
    v1.pose = {5.1, 0.0, 0.0};
    v1.idle = true;

    HungarianAssigner hungarian;
    const auto h = hungarian.assign({t0, t1, t2}, {v0, v1});
    EXPECT_EQ(h.size(), 2u);
}

TEST(HungarianAssignerTest, EmptyInput)
{
    HungarianAssigner hungarian;
    EXPECT_TRUE(hungarian.assign({}, {}).empty());
}
