#include "domain/planning/HybridAStarPlanner.h"

#include <cmath>

namespace fleetsim::domain::planning {

HybridAStarPlanner::HybridAStarPlanner(double wheelbase_m,
                                       double max_steering_rad,
                                       double motion_resolution_m,
                                       int analytic_expand_every_n)
    : wheelbase_m_(wheelbase_m)
    , max_steering_rad_(max_steering_rad)
    , motion_resolution_m_(motion_resolution_m)
    , analytic_expand_every_n_(analytic_expand_every_n)
{
}

double HybridAStarPlanner::minTurningRadiusM() const
{
    const double tan_steer = std::tan(max_steering_rad_);
    if (std::abs(tan_steer) < 1e-9 || wheelbase_m_ <= 0.0) {
        return 1e9;
    }
    return wheelbase_m_ / tan_steer;
}

core::Path HybridAStarPlanner::plan(const map::OccupancyGrid& /*grid*/,
                                    const core::Pose& /*start*/,
                                    const core::Pose& /*goal*/) const
{
    // Session 0 stub: intentionally empty so red-light tests fail until Session 1.
    // Real implementation must expand (x,y,θ) with bicycle kinematics + Dubins MVP (ADR-011).
    return {};
}

}  // namespace fleetsim::domain::planning
