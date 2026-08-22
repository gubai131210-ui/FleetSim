#pragma once

#include "Pose.h"
#include "Task.h"
#include "VehicleId.h"

namespace fleetsim::core {

struct VehicleState {
    VehicleId id;
    Pose pose;
    double linear_velocity{0.0};
    TaskId active_task_id;
    int path_index{0};
    bool idle{true};
};

}  // namespace fleetsim::core
