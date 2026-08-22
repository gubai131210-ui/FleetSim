#pragma once

#include "core/types/Task.h"
#include "core/types/VehicleId.h"
#include "core/types/VehicleState.h"

#include <vector>

namespace fleetsim::domain::scheduling {

struct TaskAssignment {
    core::TaskId task_id;
    core::VehicleId vehicle_id;
};

class ITaskAssigner {
public:
    virtual ~ITaskAssigner() = default;

    virtual std::vector<TaskAssignment> assign(
        const std::vector<core::Task>& pending_tasks,
        const std::vector<core::VehicleState>& idle_vehicles) const = 0;
};

}  // namespace fleetsim::domain::scheduling
