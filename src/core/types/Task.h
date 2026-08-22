#pragma once

#include "Pose.h"
#include "VehicleId.h"

#include <string>

namespace fleetsim::core {

using TaskId = std::string;

enum class TaskStatus {
    Pending,
    Assigned,
    Done,
};

struct Task {
    TaskId id;
    Pose pickup;
    Pose dropoff;
    int priority{0};
    TaskStatus status{TaskStatus::Pending};
    VehicleId assigned_vehicle_id;
};

}  // namespace fleetsim::core
