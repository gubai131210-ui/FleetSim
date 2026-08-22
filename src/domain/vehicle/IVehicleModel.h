#pragma once

#include "core/types/ControlCommand.h"
#include "core/types/Pose.h"

namespace fleetsim::domain::vehicle {

// Strategy interface for vehicle kinematics (DiffDrive / Bicycle).
class IVehicleModel {
public:
    virtual ~IVehicleModel() = default;

    virtual core::Pose integrate(const core::Pose& pose,
                                 const core::ControlCommand& command,
                                 double dt) const = 0;
};

}  // namespace fleetsim::domain::vehicle
