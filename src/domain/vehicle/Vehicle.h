#pragma once

#include "IVehicleModel.h"

#include "core/types/VehicleId.h"

#include <memory>
#include <string>

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

    // Kinematic metadata for trackers / scenario (not part of integrate()).
    void setModelKind(std::string kind);
    const std::string& modelKind() const { return model_kind_; }
    bool isBicycle() const { return model_kind_ == "bicycle"; }

    void setWheelbaseM(double wheelbase_m) { wheelbase_m_ = wheelbase_m; }
    double wheelbaseM() const { return wheelbase_m_; }

    void setMaxSteeringRad(double max_steering_rad) { max_steering_rad_ = max_steering_rad; }
    double maxSteeringRad() const { return max_steering_rad_; }

    void integrate(const core::ControlCommand& command, double dt);

private:
    core::VehicleId id_;
    double length_m_{1.0};
    core::Pose pose_;
    std::unique_ptr<IVehicleModel> model_;
    std::string model_kind_{"diff_drive"};
    double wheelbase_m_{0.0};
    double max_steering_rad_{0.6};
};

}  // namespace fleetsim::domain::vehicle
