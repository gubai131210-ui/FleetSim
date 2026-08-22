#include "PurePursuitTracker.h"

#include "SteeringAdapter.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace fleetsim::domain::control {

namespace {
constexpr double kPi = 3.14159265358979323846;
}  // namespace

PurePursuitTracker::PurePursuitTracker(double lookahead_m,
                                       double max_linear_velocity,
                                       double max_angular_velocity,
                                       double rotate_in_place_threshold_rad,
                                       double goal_tolerance_m)
    : lookahead_m_(lookahead_m)
    , max_linear_velocity_(max_linear_velocity)
    , max_angular_velocity_(max_angular_velocity)
    , rotate_in_place_threshold_rad_(rotate_in_place_threshold_rad)
    , goal_tolerance_m_(goal_tolerance_m)
{
}

double PurePursuitTracker::normalizeAngle(double angle)
{
    while (angle > kPi) {
        angle -= 2.0 * kPi;
    }
    while (angle < -kPi) {
        angle += 2.0 * kPi;
    }
    return angle;
}

double PurePursuitTracker::distance(const core::Pose& a, const core::Waypoint& b)
{
    const double dx = b.x - a.x;
    const double dy = b.y - a.y;
    return std::sqrt(dx * dx + dy * dy);
}

core::Waypoint PurePursuitTracker::findLookaheadPoint(const core::Pose& pose,
                                                      const core::Path& path,
                                                      double lookahead_m)
{
    const auto& points = path.waypoints();
    if (points.empty()) {
        return {};
    }

    core::Waypoint best = points.back();
    double best_delta = std::numeric_limits<double>::max();

    for (const core::Waypoint& point : points) {
        const double dx = point.x - pose.x;
        const double dy = point.y - pose.y;
        const double forward_x = std::cos(pose.theta);
        const double forward_y = std::sin(pose.theta);
        if (dx * forward_x + dy * forward_y <= 0.0) {
            continue;
        }

        const double dist = distance(pose, point);
        const double delta = std::abs(dist - lookahead_m);
        if (dist >= lookahead_m * 0.5 && delta < best_delta) {
            best_delta = delta;
            best = point;
        }
    }

    return best;
}

core::ControlCommand PurePursuitTracker::compute(const core::Pose& current_pose,
                                                 const core::Path& reference_path,
                                                 double /*dt*/) const
{
    return computeInternal(current_pose, reference_path, 0.0, 0.6);
}

core::ControlCommand PurePursuitTracker::compute(const core::Pose& current_pose,
                                                 const core::Path& reference_path,
                                                 double /*dt*/,
                                                 double bicycle_wheelbase_m,
                                                 double max_steering_rad) const
{
    return computeInternal(current_pose, reference_path, bicycle_wheelbase_m, max_steering_rad);
}

core::ControlCommand PurePursuitTracker::computeInternal(
    const core::Pose& current_pose,
    const core::Path& reference_path,
    double bicycle_wheelbase_m,
    double max_steering_rad) const
{
    core::ControlCommand command;

    if (reference_path.empty()) {
        return command;
    }

    const core::Waypoint goal = reference_path.waypoints().back();
    if (distance(current_pose, goal) <= goal_tolerance_m_) {
        return command;
    }

    const core::Waypoint lookahead =
        findLookaheadPoint(current_pose, reference_path, lookahead_m_);
    const double dx = lookahead.x - current_pose.x;
    const double dy = lookahead.y - current_pose.y;
    const double target_heading = std::atan2(dy, dx);
    const double heading_error = normalizeAngle(target_heading - current_pose.theta);
    const bool bicycle = bicycle_wheelbase_m > 0.0;

    if (std::abs(heading_error) > rotate_in_place_threshold_rad_) {
        if (bicycle) {
            // Car-like robots cannot spin in place: creep + saturated steering.
            command.linear_velocity = max_linear_velocity_ * 0.3;
            command.steering_angle =
                std::copysign(max_steering_rad, heading_error);
            command.angular_velocity = 0.0;
        } else {
            command.linear_velocity = 0.0;
            command.angular_velocity =
                std::clamp(heading_error, -max_angular_velocity_, max_angular_velocity_);
        }
        return command;
    }

    const double curvature =
        2.0 * std::sin(heading_error) / std::max(lookahead_m_, 1e-3);
    command.linear_velocity = max_linear_velocity_;
    command.angular_velocity = std::clamp(curvature * command.linear_velocity,
                                          -max_angular_velocity_,
                                          max_angular_velocity_);

    if (bicycle) {
        SteeringAdapter::applyCurvatureToCommand(
            command, curvature, bicycle_wheelbase_m, max_steering_rad);
    }

    return command;
}

}  // namespace fleetsim::domain::control
