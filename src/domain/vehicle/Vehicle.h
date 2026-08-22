#pragma once

#include "DiffDriveModel.h"

#include "core/types/VehicleId.h"

namespace fleetsim::domain::vehicle {

class Vehicle {
public:
    Vehicle(core::VehicleId id, double length_m, const core::Pose& initial_pose);

    const core::VehicleId& id() const { return id_; }
    double lengthM() const { return length_m_; }
    const core::Pose& pose() const { return pose_; }

    void setPose(const core::Pose& pose) { pose_ = pose; }

    void integrate(const core::ControlCommand& command, double dt);

private:
    core::VehicleId id_;
    double length_m_{1.0};
    core::Pose pose_;
    DiffDriveModel model_;
};

}  // namespace fleetsim::domain::vehicle
