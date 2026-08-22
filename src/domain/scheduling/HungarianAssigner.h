#pragma once

#include "ITaskAssigner.h"

namespace fleetsim::domain::scheduling {

// Hungarian (Munkres) global assignment on pickup-distance cost matrix.
// Rectangular cases are padded with virtual nodes (zero/large cost) as needed.
class HungarianAssigner : public ITaskAssigner {
public:
    std::vector<TaskAssignment> assign(
        const std::vector<core::Task>& pending_tasks,
        const std::vector<core::VehicleState>& idle_vehicles) const override;
};

}  // namespace fleetsim::domain::scheduling
