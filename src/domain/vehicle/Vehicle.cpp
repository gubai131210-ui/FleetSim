#include "Vehicle.h"

#include "DiffDriveModel.h"

#include <utility>

namespace fleetsim::domain::vehicle {

namespace {

std::unique_ptr<IVehicleModel> defaultOr(std::unique_ptr<IVehicleModel> model)
{
    if (model) {
        return model;
    }
    return std::make_unique<DiffDriveModel>();
}

}  // namespace

Vehicle::Vehicle(core::VehicleId id, double length_m, const core::Pose& initial_pose)
    : id_(std::move(id))
    , length_m_(length_m)
    , pose_(initial_pose)
    , model_(std::make_unique<DiffDriveModel>())
    , model_kind_("diff_drive")
{
}

Vehicle::Vehicle(core::VehicleId id,
                 double length_m,
                 const core::Pose& initial_pose,
                 std::unique_ptr<IVehicleModel> model)
    : id_(std::move(id))
    , length_m_(length_m)
    , pose_(initial_pose)
    , model_(defaultOr(std::move(model)))
    , model_kind_("diff_drive")
{
}

void Vehicle::setModel(std::unique_ptr<IVehicleModel> model)
{
    model_ = defaultOr(std::move(model));
}

void Vehicle::setModelKind(std::string kind)
{
    if (kind.empty()) {
        model_kind_ = "diff_drive";
        return;
    }
    model_kind_ = std::move(kind);
}

void Vehicle::integrate(const core::ControlCommand& command, double dt)
{
    pose_ = model_->integrate(pose_, command, dt);
}

}  // namespace fleetsim::domain::vehicle
