#pragma once

#include "IVehicleModel.h"

#include <memory>
#include <string_view>

namespace fleetsim::domain::vehicle {

// Factory for DiffDrive / Bicycle models. Unknown or empty kind → DiffDrive.
std::unique_ptr<IVehicleModel> createVehicleModel(
    std::string_view kind,
    double max_linear_velocity = 0.5,
    double max_angular_velocity = 1.0,
    double wheelbase_m = 0.8,
    double max_steering_rad = 0.6);

}  // namespace fleetsim::domain::vehicle
