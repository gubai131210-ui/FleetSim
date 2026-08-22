#pragma once

#include "ITaskAssigner.h"

namespace fleetsim::domain::scheduling {

class GreedyAssigner : public ITaskAssigner {
public:
    std::vector<TaskAssignment> assign(
        const std::vector<core::Task>& pending_tasks,
        const std::vector<core::VehicleState>& idle_vehicles) const override;
};

}  // namespace fleetsim::domain::scheduling
