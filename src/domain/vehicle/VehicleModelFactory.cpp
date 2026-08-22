#include "VehicleModelFactory.h"

#include "BicycleModel.h"
#include "DiffDriveModel.h"

namespace fleetsim::domain::vehicle {

std::unique_ptr<IVehicleModel> createVehicleModel(std::string_view kind,
                                                  double max_linear_velocity,
                                                  double max_angular_velocity,
                                                  double wheelbase_m,
                                                  double max_steering_rad)
{
    if (kind == "bicycle") {
        return std::make_unique<BicycleModel>(wheelbase_m,
                                              max_linear_velocity,
                                              max_steering_rad);
    }

    // "diff_drive", empty, or unknown → DiffDrive (safe default).
    return std::make_unique<DiffDriveModel>(max_linear_velocity, max_angular_velocity);
}

}  // namespace fleetsim::domain::vehicle
