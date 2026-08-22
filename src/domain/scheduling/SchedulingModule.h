#pragma once

#include "GreedyAssigner.h"
#include "TaskQueue.h"
#include "vehicle/FleetManager.h"

namespace fleetsim::domain::scheduling {

class SchedulingModule {
public:
    TaskQueue& tasks();
    const TaskQueue& tasks() const;

    void loadTasks(const std::vector<core::Task>& tasks);
    void tick(double dt, vehicle::FleetManager& fleet);

    int pendingTaskCount() const;

private:
    void applyAssignment(const TaskAssignment& assignment,
                         vehicle::FleetManager& fleet);

    TaskQueue task_queue_;
    GreedyAssigner assigner_;
    double assign_cooldown_s_{0.0};
};

}  // namespace fleetsim::domain::scheduling
