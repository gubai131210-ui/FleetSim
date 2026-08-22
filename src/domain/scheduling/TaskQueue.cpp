#include "TaskQueue.h"

#include <algorithm>

namespace fleetsim::domain::scheduling {

void TaskQueue::clear()
{
    tasks_.clear();
}

void TaskQueue::addTask(const core::Task& task)
{
    tasks_.push_back(task);
}

void TaskQueue::loadTasks(const std::vector<core::Task>& tasks)
{
    tasks_ = tasks;
}

std::vector<core::Task> TaskQueue::pendingTasks() const
{
    std::vector<core::Task> pending;
    for (const core::Task& task : tasks_) {
        if (task.status == core::TaskStatus::Pending) {
            pending.push_back(task);
        }
    }
    return pending;
}

std::optional<core::Task> TaskQueue::findTask(const core::TaskId& task_id) const
{
    for (const core::Task& task : tasks_) {
        if (task.id == task_id) {
            return task;
        }
    }
    return std::nullopt;
}

bool TaskQueue::markAssigned(const core::TaskId& task_id, const core::VehicleId& vehicle_id)
{
    core::Task* task = findMutable(task_id);
    if (task == nullptr || task->status != core::TaskStatus::Pending) {
        return false;
    }
    task->status = core::TaskStatus::Assigned;
    task->assigned_vehicle_id = vehicle_id;
    return true;
}

bool TaskQueue::markDone(const core::TaskId& task_id)
{
    core::Task* task = findMutable(task_id);
    if (task == nullptr) {
        return false;
    }
    task->status = core::TaskStatus::Done;
    return true;
}

int TaskQueue::pendingCount() const
{
    return static_cast<int>(std::count_if(tasks_.begin(), tasks_.end(), [](const core::Task& task) {
        return task.status == core::TaskStatus::Pending;
    }));
}

int TaskQueue::assignedCount() const
{
    return static_cast<int>(std::count_if(tasks_.begin(), tasks_.end(), [](const core::Task& task) {
        return task.status == core::TaskStatus::Assigned;
    }));
}

int TaskQueue::doneCount() const
{
    return static_cast<int>(std::count_if(tasks_.begin(), tasks_.end(), [](const core::Task& task) {
        return task.status == core::TaskStatus::Done;
    }));
}

core::Task* TaskQueue::findMutable(const core::TaskId& task_id)
{
    for (core::Task& task : tasks_) {
        if (task.id == task_id) {
            return &task;
        }
    }
    return nullptr;
}

}  // namespace fleetsim::domain::scheduling
