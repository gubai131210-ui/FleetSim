#pragma once

#include "IVehicleModel.h"

#include "core/types/VehicleId.h"

#include <memory>

namespace fleetsim::domain::vehicle {

class Vehicle {
public:
    // Default kinematics: DiffDrive (Phase 1–3 compatibility).
    Vehicle(core::VehicleId id, double length_m, const core::Pose& initial_pose);

    // Takes ownership of model; nullptr falls back to DiffDrive.
    Vehicle(core::VehicleId id,
            double length_m,
            const core::Pose& initial_pose,
            std::unique_ptr<IVehicleModel> model);

    const core::VehicleId& id() const { return id_; }
    double lengthM() const { return length_m_; }
    const core::Pose& pose() const { return pose_; }

    void setPose(const core::Pose& pose) { pose_ = pose; }

    void setModel(std::unique_ptr<IVehicleModel> model);

    void integrate(const core::ControlCommand& command, double dt);

private:
    core::VehicleId id_;
    double length_m_{1.0};
    core::Pose pose_;
    std::unique_ptr<IVehicleModel> model_;
};

}  // namespace fleetsim::domain::vehicle
