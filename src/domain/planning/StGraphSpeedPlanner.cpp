#include "domain/planning/StGraphSpeedPlanner.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace fleetsim::domain::planning {

namespace {

constexpr double kLateralHitM = 0.75;
constexpr double kSBufferM = 0.6;
constexpr double kTHalfWindowS = 1.5;
constexpr double kMinSpeedFloor = 0.05;

}  // namespace

StGraphSpeedPlanner::StGraphSpeedPlanner(double v_max, double a_max, double dt_grid)
    : v_max_(v_max)
    , a_max_(a_max)
    , dt_grid_(dt_grid)
{
}

std::vector<double> StGraphSpeedPlanner::cumulativeArcLength(const core::Path& path)
{
    std::vector<double> s(path.size(), 0.0);
    if (path.empty()) {
        return s;
    }
    const auto& w = path.waypoints();
    for (std::size_t i = 1; i < w.size(); ++i) {
        const double dx = w[i].x - w[i - 1].x;
        const double dy = w[i].y - w[i - 1].y;
        s[i] = s[i - 1] + std::sqrt(dx * dx + dy * dy);
    }
    return s;
}

bool StGraphSpeedPlanner::projectOntoPath(const core::Path& path,
                                          double x,
                                          double y,
                                          double* s_out,
                                          double* lateral_out)
{
    if (path.size() < 2 || s_out == nullptr || lateral_out == nullptr) {
        return false;
    }
    const auto& w = path.waypoints();
    double best_d2 = std::numeric_limits<double>::infinity();
    double best_s = 0.0;
    double best_lat = 0.0;
    double s_acc = 0.0;
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
        const double seg_len = std::sqrt(ab2);
        double t = ((x - ax) * abx + (y - ay) * aby) / ab2;
        t = std::max(0.0, std::min(1.0, t));
        const double qx = ax + t * abx;
        const double qy = ay + t * aby;
        const double dx = x - qx;
        const double dy = y - qy;
        const double d2 = dx * dx + dy * dy;
        if (d2 < best_d2) {
            best_d2 = d2;
            best_s = s_acc + t * seg_len;
            best_lat = (abx * dy - aby * dx) / seg_len;
        }
        s_acc += seg_len;
    }
    if (!std::isfinite(best_d2)) {
        return false;
    }
    *s_out = best_s;
    *lateral_out = best_lat;
    return true;
}

std::vector<StGraphSpeedPlanner::Occupancy> StGraphSpeedPlanner::buildOccupancies(
    const core::Path& ego_path,
    const std::vector<double>& ego_s,
    const std::vector<PeerTrajectory>& peers) const
{
    std::vector<Occupancy> out;
    if (ego_path.empty() || ego_s.empty()) {
        return out;
    }
    const double s_end = ego_s.back();
    const double t_half = std::max(kTHalfWindowS, 2.0 * dt_grid_);

    for (const PeerTrajectory& peer : peers) {
        if (peer.path.size() < 2) {
            continue;
        }
        const double v_nom = std::max(peer.nominal_speed, 1e-3);
        const auto peer_s = cumulativeArcLength(peer.path);
        const auto& pw = peer.path.waypoints();

        // Dense samples along peer polyline (waypoints + mid-edges).
        for (std::size_t i = 0; i < pw.size(); ++i) {
            double s_hit = 0.0;
            double lat = 0.0;
            if (!projectOntoPath(ego_path, pw[i].x, pw[i].y, &s_hit, &lat)) {
                continue;
            }
            if (std::abs(lat) > kLateralHitM) {
                continue;
            }
            if (s_hit < -1e-6 || s_hit > s_end + 1e-6) {
                continue;
            }
            const double t_peer = peer_s[i] / v_nom;
            Occupancy occ;
            occ.s_min = std::max(0.0, s_hit - kSBufferM);
            occ.s_max = std::min(s_end, s_hit + kSBufferM);
            occ.t_min = std::max(0.0, t_peer - t_half);
            occ.t_max = t_peer + t_half;
            out.push_back(occ);
        }
    }
    return out;
}

core::SpeedProfile StGraphSpeedPlanner::plan(const core::Path& ego_path,
                                             const std::vector<PeerTrajectory>& peers) const
{
    core::SpeedProfile profile;
    if (ego_path.empty()) {
        return profile;
    }

    const std::size_t n = ego_path.size();
    const auto ego_s = cumulativeArcLength(ego_path);
    const double v_cruise = std::max(v_max_, 1e-6);

    profile.speeds.assign(n, v_cruise);
    profile.arrival_times.assign(n, 0.0);

    const std::vector<Occupancy> occs = buildOccupancies(ego_path, ego_s, peers);

    // Free-flow times; then push arrival past any ST block that free-flow hits.
    std::vector<double> t_arrive(n, 0.0);
    for (std::size_t i = 1; i < n; ++i) {
        t_arrive[i] = ego_s[i] / v_cruise;
    }

    for (const Occupancy& occ : occs) {
        // Free-flow curve s |-> s/v intersects [s_min,s_max] x [t_min,t_max]?
        const double t_enter = occ.s_min / v_cruise;
        const double t_exit = occ.s_max / v_cruise;
        const bool overlaps =
            !(t_exit < occ.t_min - 1e-9 || t_enter > occ.t_max + 1e-9);
        if (!overlaps) {
            continue;
        }
        // Wait until block clears: arrive at s_min no earlier than t_max.
        const double t_clear = occ.t_max + dt_grid_;
        if (t_clear <= 1e-9) {
            continue;
        }
        const double v_need = occ.s_min / t_clear;
        const double v_cap = std::clamp(v_need, kMinSpeedFloor, v_cruise);

        // Apply continuous deceleration into the block (not a single hard zero).
        for (std::size_t i = 0; i < n; ++i) {
            const double s = ego_s[i];
            if (s <= occ.s_min) {
                // Ramp from cruise at s=0 toward v_cap at s_min.
                const double alpha = (occ.s_min > 1e-9) ? (s / occ.s_min) : 1.0;
                const double v_i = v_cruise + alpha * (v_cap - v_cruise);
                profile.speeds[i] = std::min(profile.speeds[i], v_i);
            } else if (s <= occ.s_max) {
                profile.speeds[i] = std::min(profile.speeds[i], v_cap);
            } else {
                // Recover toward cruise after the block.
                const double span = std::max(ego_s.back() - occ.s_max, 1e-9);
                const double beta = std::min(1.0, (s - occ.s_max) / span);
                const double v_i = v_cap + beta * (v_cruise - v_cap);
                profile.speeds[i] = std::min(profile.speeds[i], v_i);
            }
        }
    }

    // Rebuild arrival times from piecewise speeds (use segment average).
    profile.arrival_times[0] = 0.0;
    for (std::size_t i = 1; i < n; ++i) {
        const double ds = ego_s[i] - ego_s[i - 1];
        const double v_seg =
            std::max(0.5 * (profile.speeds[i - 1] + profile.speeds[i]), kMinSpeedFloor);
        profile.arrival_times[i] = profile.arrival_times[i - 1] + ds / v_seg;
    }

    // Soft accel limit: if adjacent Δv too large, pull toward previous (teaching).
    if (a_max_ > 1e-9 && n >= 2) {
        for (std::size_t i = 1; i < n; ++i) {
            const double ds = std::max(ego_s[i] - ego_s[i - 1], 1e-6);
            const double dt = ds / std::max(profile.speeds[i - 1], kMinSpeedFloor);
            const double dv_max = a_max_ * dt;
            const double lo = profile.speeds[i - 1] - dv_max;
            const double hi = profile.speeds[i - 1] + dv_max;
            profile.speeds[i] = std::clamp(profile.speeds[i], std::max(lo, kMinSpeedFloor),
                                           std::min(hi, v_cruise));
        }
        profile.arrival_times[0] = 0.0;
        for (std::size_t i = 1; i < n; ++i) {
            const double ds = ego_s[i] - ego_s[i - 1];
            const double v_seg =
                std::max(0.5 * (profile.speeds[i - 1] + profile.speeds[i]), kMinSpeedFloor);
            profile.arrival_times[i] = profile.arrival_times[i - 1] + ds / v_seg;
        }
    }

    return profile;
}

}  // namespace fleetsim::domain::planning
