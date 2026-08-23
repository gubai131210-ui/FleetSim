#include "domain/control/MpcLateralTracker.h"

#include "domain/control/DenseQpSolver.h"

#include <Eigen/Dense>

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace fleetsim::domain::control {

namespace {

constexpr double kPi = 3.14159265358979323846;
constexpr double kTwoPi = 2.0 * kPi;
constexpr double kEpsV = 1e-3;
constexpr int kMaxHorizon = 20;

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

struct PathError {
    double e{0.0};       // signed lateral (left of path positive, path×r)
    double theta_e{0.0}; // wrap(theta - path_heading)
    bool ok{false};
};

// Rear-axle cross-track + heading vs nearest path segment (κ frozen 0 in MVP).
PathError computePathError(const core::Pose& pose, const core::Path& path)
{
    PathError out;
    if (path.size() < 2) {
        return out;
    }
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
        const double apx = pose.x - ax;
        const double apy = pose.y - ay;
        double t = (apx * abx + apy * aby) / ab2;
        t = std::max(0.0, std::min(1.0, t));
        const double qx = ax + t * abx;
        const double qy = ay + t * aby;
        const double dx = pose.x - qx;
        const double dy = pose.y - qy;
        const double dist2 = dx * dx + dy * dy;
        if (dist2 < best_dist2) {
            best_dist2 = dist2;
            const double tang_len = std::sqrt(ab2);
            const double path_heading = std::atan2(aby, abx);
            // +e when vehicle is left of directed path (ab × r).
            out.e = (abx * dy - aby * dx) / tang_len;
            out.theta_e = wrapToPi(pose.theta - path_heading);
            out.ok = true;
        }
    }
    return out;
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
    last_solve_ok_ = false;
    last_prediction_norm_ = 0.0;
    last_cost_nontrivial_ = false;

    core::ControlCommand cmd;
    cmd.angular_velocity = 0.0;
    const double v_cmd = resolveTargetSpeed(current_pose, reference_path);
    cmd.linear_velocity = std::isfinite(v_cmd) ? v_cmd : 0.0;
    cmd.steering_angle = 0.0;

    // Failure path: illegal horizon / dt / geometry (ADR-014).
    if (horizon_ < 1 || horizon_ > kMaxHorizon || !(mpc_dt_ > 0.0) ||
        !(wheelbase_m_ > 1e-6) || !(max_steering_rad_ > 0.0) ||
        reference_path.size() < 2) {
        return cmd;
    }

    const PathError err = computePathError(current_pose, reference_path);
    if (!err.ok || !std::isfinite(err.e) || !std::isfinite(err.theta_e)) {
        return cmd;
    }

    const double v = std::copysign(std::max(std::abs(cmd.linear_velocity), kEpsV),
                                   (cmd.linear_velocity >= 0.0) ? 1.0 : -1.0);
    const double L = wheelbase_m_;
    const double Ts = mpc_dt_;
    const int N = horizon_;

    // Discrete LTI: x=[e, θe], u=δ; κ frozen 0.
    // A = [[1, v Ts],[0, 1]], B = [[0], [(v/L) Ts]]
    Eigen::Matrix2d A;
    A << 1.0, v * Ts, 0.0, 1.0;
    Eigen::Vector2d B;
    B << 0.0, (v / L) * Ts;

    last_prediction_norm_ = A.norm() + B.norm();
    last_cost_nontrivial_ =
        (q_lat_ > 0.0 && q_heading_ > 0.0 && r_steer_ > 0.0 && last_prediction_norm_ > 1e-6);

    Eigen::Matrix2d Q = Eigen::Matrix2d::Zero();
    Q(0, 0) = q_lat_;
    Q(1, 1) = q_heading_;
    const Eigen::Matrix2d Qf = Q;
    const double R = r_steer_;

    Eigen::Vector2d x0;
    x0 << err.e, err.theta_e;

    // Condensed QP: stacked X = [x1..xN] = Sx x0 + Su U
    // x_{k+1} = A^{k+1} x0 + sum_{j=0}^{k} A^{k-j} B u_j
    Eigen::MatrixXd Sx = Eigen::MatrixXd::Zero(2 * N, 2);
    Eigen::MatrixXd Su = Eigen::MatrixXd::Zero(2 * N, N);
    std::vector<Eigen::Matrix2d> Apow(static_cast<std::size_t>(N) + 1);
    Apow[0] = Eigen::Matrix2d::Identity();
    for (int i = 1; i <= N; ++i) {
        Apow[static_cast<std::size_t>(i)] = A * Apow[static_cast<std::size_t>(i - 1)];
    }
    for (int k = 0; k < N; ++k) {
        Sx.block<2, 2>(2 * k, 0) = Apow[static_cast<std::size_t>(k + 1)];
        for (int j = 0; j <= k; ++j) {
            Su.block<2, 1>(2 * k, j) = Apow[static_cast<std::size_t>(k - j)] * B;
        }
    }

    Eigen::MatrixXd Qbar = Eigen::MatrixXd::Zero(2 * N, 2 * N);
    for (int k = 0; k < N; ++k) {
        Qbar.block<2, 2>(2 * k, 2 * k) = (k == N - 1) ? Qf : Q;
    }
    Eigen::MatrixXd Rbar = Eigen::MatrixXd::Identity(N, N) * R;

    // J = (Sx x0 + Su U)' Qbar (Sx x0 + Su U) + U' Rbar U
    //   = 0.5 U' (2 Su' Qbar Su + 2 Rbar) U + (2 Su' Qbar Sx x0)' U + const
    // Use H = 2(Su'Q Su + R), g = 2 Su' Q Sx x0  for 0.5 U'H U + g'U
    const Eigen::MatrixXd H = 2.0 * (Su.transpose() * Qbar * Su + Rbar);
    const Eigen::VectorXd g = 2.0 * (Su.transpose() * Qbar * Sx * x0);

    Eigen::VectorXd lo = Eigen::VectorXd::Constant(N, -max_steering_rad_);
    Eigen::VectorXd hi = Eigen::VectorXd::Constant(N, max_steering_rad_);

    const DenseQpSolver::Result qp = DenseQpSolver::solveBox(H, g, lo, hi);
    if (!qp.ok || qp.u.size() != N || !qp.u.allFinite()) {
        return cmd;
    }

    double u0 = qp.u(0);
    u0 = std::clamp(u0, -max_steering_rad_, max_steering_rad_);
    if (!std::isfinite(u0)) {
        return cmd;
    }

    cmd.steering_angle = u0;
    last_solve_ok_ = true;

    // DiffDrive compatibility: ω ≈ (v/L) tanδ
    if (L > 1e-6) {
        cmd.angular_velocity = (cmd.linear_velocity / L) * std::tan(u0);
    }
    return cmd;
}

}  // namespace fleetsim::domain::control
