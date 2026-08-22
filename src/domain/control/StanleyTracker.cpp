#include "domain/control/StanleyTracker.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace fleetsim::domain::control {

namespace {

constexpr double kPi = 3.14159265358979323846;
constexpr double kTwoPi = 2.0 * kPi;

double wrapToPi(double a)
{
    while (a > kPi) {
        a -= kTwoPi;
    }
    while (a < -kPi) {
        a += kTwoPi;
    }
    return a;
}

struct NearestSegment {
    double path_heading{0.0};
    double signed_cross{0.0};  // + when front axle is left of path (ab × r)
    bool ok{false};
};

NearestSegment findNearest(const core::Pose& pose,
                           double wheelbase_m,
                           const core::Path& path)
{
    NearestSegment best;
    if (path.size() < 2 || wheelbase_m < 0.0) {
        return best;
    }

    const double fx = pose.x + wheelbase_m * std::cos(pose.theta);
    const double fy = pose.y + wheelbase_m * std::sin(pose.theta);
    double best_dist2 = std::numeric_limits<double>::infinity();
    const auto& w = path.waypoints();

    for (std::size_t i = 0; i + 1 < w.size(); ++i) {
        const double ax = w[i].x;
        const double ay = w[i].y;
        const double bx = w[i + 1].x;
        const double by = w[i + 1].y;
        const double abx = bx - ax;
        const double aby = by - ay;
        const double ab2 = abx * abx + aby * aby;
        if (ab2 < 1e-12) {
            continue;
        }
        const double apx = fx - ax;
        const double apy = fy - ay;
        double t = (apx * abx + apy * aby) / ab2;
        t = std::max(0.0, std::min(1.0, t));
        const double qx = ax + t * abx;
        const double qy = ay + t * aby;
        const double dx = fx - qx;
        const double dy = fy - qy;
        const double dist2 = dx * dx + dy * dy;
        if (dist2 < best_dist2) {
            best_dist2 = dist2;
            const double tang_len = std::sqrt(ab2);
            best.path_heading = std::atan2(aby, abx);
            best.signed_cross = (abx * dy - aby * dx) / tang_len;
            best.ok = true;
        }
    }
    return best;
}

}  // namespace

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

core::ControlCommand StanleyTracker::compute(const core::Pose& current_pose,
                                             const core::Path& reference_path,
                                             double /*dt*/) const
{
    core::ControlCommand command;
    if (reference_path.size() < 2) {
        return command;
    }

    const NearestSegment nearest = findNearest(current_pose, wheelbase_m_, reference_path);
    if (!nearest.ok) {
        return command;
    }

    // Classic Stanley (Snider): δ = θe + arctan(k e / (v+ε))
    // θe = vehicle heading − path heading; e > 0 when vehicle is to the right of path.
    const double theta_e = wrapToPi(current_pose.theta - nearest.path_heading);
    const double e = -nearest.signed_cross;
    const double v_soft = std::max(std::abs(cruise_velocity_), 0.0) + softening_v_;
    double steering = theta_e + std::atan2(k_gain_ * e, v_soft);
    steering = std::clamp(steering, -max_steering_rad_, max_steering_rad_);

    command.steering_angle = steering;
    command.linear_velocity = cruise_velocity_;
    command.angular_velocity = 0.0;

    // DiffDrive compatibility: ω ≈ (v/L) tanδ when wheelbase known.
    if (wheelbase_m_ > 1e-6) {
        command.angular_velocity =
            (command.linear_velocity / wheelbase_m_) * std::tan(steering);
    }
    return command;
}

}  // namespace fleetsim::domain::control
