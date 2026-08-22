#pragma once

#include "core/types/Task.h"

#include <optional>
#include <vector>

namespace fleetsim::domain::scheduling {

class TaskQueue {
public:
    void clear();
    void addTask(const core::Task& task);
    void loadTasks(const std::vector<core::Task>& tasks);

    const std::vector<core::Task>& tasks() const { return tasks_; }

    std::vector<core::Task> pendingTasks() const;
    std::optional<core::Task> findTask(const core::TaskId& task_id) const;

    bool markAssigned(const core::TaskId& task_id, const core::VehicleId& vehicle_id);
    bool markDone(const core::TaskId& task_id);

    int pendingCount() const;
    int assignedCount() const;
    int doneCount() const;

private:
    core::Task* findMutable(const core::TaskId& task_id);

    std::vector<core::Task> tasks_;
};

}  // namespace fleetsim::domain::scheduling
