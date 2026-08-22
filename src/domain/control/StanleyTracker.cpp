#include "domain/control/StanleyTracker.h"

namespace fleetsim::domain::control {

StanleyTracker::StanleyTracker(double k_gain,
                               double softening_v,
                               double max_steering_rad,
                               double wheelbase_m,
                               double cruise_velocity)
    : k_gain_(k_gain)
    , softening_v_(softening_v)
    , max_steering_rad_(max_steering_rad)
    , wheelbase_m_(wheelbase_m)
    , cruise_velocity_(cruise_velocity)
{
}

core::ControlCommand StanleyTracker::compute(const core::Pose& /*current_pose*/,
                                             const core::Path& /*reference_path*/,
                                             double /*dt*/) const
{
    // Session 0 stub: zero command so red-light tests fail until Session 3.
    // Real implementation must use front-axle cross-track e and heading error θe (ADR-012).
    return {};
}

}  // namespace fleetsim::domain::control
