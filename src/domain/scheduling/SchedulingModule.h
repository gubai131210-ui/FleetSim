#pragma once

#include "GreedyAssigner.h"
#include "ITaskAssigner.h"
#include "TaskQueue.h"
#include "vehicle/FleetManager.h"

#include <memory>

namespace fleetsim::domain::scheduling {

class SchedulingModule {
public:
    SchedulingModule();

    explicit SchedulingModule(std::unique_ptr<ITaskAssigner> assigner);

    void setAssigner(std::unique_ptr<ITaskAssigner> assigner);

    TaskQueue& tasks();
    const TaskQueue& tasks() const;

    void loadTasks(const std::vector<core::Task>& tasks);
    void tick(double dt, vehicle::FleetManager& fleet);

    int pendingTaskCount() const;

private:
    void applyAssignment(const TaskAssignment& assignment,
                         vehicle::FleetManager& fleet);

    TaskQueue task_queue_;
    std::unique_ptr<ITaskAssigner> assigner_;
    double assign_cooldown_s_{0.0};
};

}  // namespace fleetsim::domain::scheduling
