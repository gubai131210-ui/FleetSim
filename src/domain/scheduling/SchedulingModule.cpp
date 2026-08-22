#include "SchedulingModule.h"

namespace fleetsim::domain::scheduling {

TaskQueue& SchedulingModule::tasks()
{
    return task_queue_;
}

const TaskQueue& SchedulingModule::tasks() const
{
    return task_queue_;
}

void SchedulingModule::loadTasks(const std::vector<core::Task>& tasks)
{
    task_queue_.loadTasks(tasks);
}

void SchedulingModule::applyAssignment(const TaskAssignment& assignment,
                                         vehicle::FleetManager& fleet)
{
    const auto task = task_queue_.findTask(assignment.task_id);
    if (!task.has_value()) {
        return;
    }
    if (!task_queue_.markAssigned(assignment.task_id, assignment.vehicle_id)) {
        return;
    }
    fleet.startTask(assignment.vehicle_id, task.value());
}

void SchedulingModule::tick(double dt, vehicle::FleetManager& fleet)
{
    assign_cooldown_s_ -= dt;
    if (assign_cooldown_s_ > 0.0) {
        return;
    }

    const std::vector<core::Task> pending = task_queue_.pendingTasks();
    const std::vector<core::VehicleState> idle_vehicles = fleet.idleVehicleStates();
    const std::vector<TaskAssignment> assignments = assigner_.assign(pending, idle_vehicles);
    for (const TaskAssignment& assignment : assignments) {
        applyAssignment(assignment, fleet);
    }

    if (!assignments.empty()) {
        assign_cooldown_s_ = 0.1;
    }
}

int SchedulingModule::pendingTaskCount() const
{
    return task_queue_.pendingCount();
}

}  // namespace fleetsim::domain::scheduling
