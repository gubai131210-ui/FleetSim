#include "domain/control/MpcLateralTracker.h"

#include <cmath>
#include <limits>

namespace fleetsim::domain::control {

namespace {

std::size_t nearestWaypointIndex(const core::Pose& pose, const core::Path& path)
{
    if (path.empty()) {
        return 0;
    }
    std::size_t best = 0;
    double best_d2 = std::numeric_limits<double>::infinity();
    const auto& w = path.waypoints();
    for (std::size_t i = 0; i < w.size(); ++i) {
        const double dx = pose.x - w[i].x;
        const double dy = pose.y - w[i].y;
        const double d2 = dx * dx + dy * dy;
        if (d2 < best_d2) {
            best_d2 = d2;
            best = i;
        }
    }
    return best;
}

}  // namespace

MpcLateralTracker::MpcLateralTracker(int horizon,
                                     double mpc_dt,
                                     double q_lat,
                                     double q_heading,
                                     double r_steer,
                                     double max_steering_rad,
                                     double wheelbase_m,
                                     double cruise_speed)
    : horizon_(horizon)
    , mpc_dt_(mpc_dt)
    , q_lat_(q_lat)
    , q_heading_(q_heading)
    , r_steer_(r_steer)
    , max_steering_rad_(max_steering_rad)
    , wheelbase_m_(wheelbase_m)
    , cruise_speed_(cruise_speed)
{
}

void MpcLateralTracker::setSpeedProfile(const core::SpeedProfile* profile)
{
    speed_profile_ = profile;
}

double MpcLateralTracker::resolveTargetSpeed(const core::Pose& pose,
                                             const core::Path& path) const
{
    if (speed_profile_ == nullptr || path.empty()) {
        return cruise_speed_;
    }
    if (speed_profile_->speeds.size() != path.size()) {
        return cruise_speed_;
    }
    const std::size_t i = nearestWaypointIndex(pose, path);
    if (i >= speed_profile_->speeds.size()) {
        return cruise_speed_;
    }
    return speed_profile_->speeds[i];
}

core::ControlCommand MpcLateralTracker::compute(const core::Pose& current_pose,
                                                const core::Path& reference_path,
                                                double /*dt*/) const
{
    // Session 0 stub: honest non-MPC. Zero steer; velocity from profile/cruise.
    // Session 1 replaces with linearized error dynamics + Eigen dense QP.
    last_solve_ok_ = false;

    core::ControlCommand cmd;
    cmd.steering_angle = 0.0;
    cmd.angular_velocity = 0.0;
    cmd.linear_velocity = resolveTargetSpeed(current_pose, reference_path);
    if (!std::isfinite(cmd.linear_velocity)) {
        cmd.linear_velocity = 0.0;
    }
    return cmd;
}

}  // namespace fleetsim::domain::control
