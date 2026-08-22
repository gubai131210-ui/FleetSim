#include "domain/planning/HybridAStarPlanner.h"

#include "domain/planning/DubinsPath.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <queue>
#include <unordered_map>
#include <vector>

namespace fleetsim::domain::planning {

namespace {

constexpr double kPi = 3.14159265358979323846;
constexpr double kTwoPi = 2.0 * kPi;
constexpr int kHeadingBins = 32;
constexpr int kMaxExpansions = 40000;

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

int headingBin(double theta)
{
    double a = std::fmod(theta, kTwoPi);
    if (a < 0.0) {
        a += kTwoPi;
    }
    int bin = static_cast<int>(std::floor(a / kTwoPi * kHeadingBins));
    if (bin >= kHeadingBins) {
        bin = kHeadingBins - 1;
    }
    if (bin < 0) {
        bin = 0;
    }
    return bin;
}

std::int64_t stateKey(int ix, int iy, int itheta)
{
    // Pack into 64 bits: 24+24+8 roughly.
    return (static_cast<std::int64_t>(ix & 0xFFFFFF) << 32)
        ^ (static_cast<std::int64_t>(iy & 0xFFFFFF) << 8)
        ^ static_cast<std::int64_t>(itheta & 0xFF);
}

struct SearchNode {
    double x{0.0};
    double y{0.0};
    double theta{0.0};
    double g{0.0};
    double f{0.0};
    int parent{-1};
};

struct OpenItem {
    double f{0.0};
    int index{0};

    bool operator>(const OpenItem& other) const { return f > other.f; }
};

bool cellFree(const map::OccupancyGrid& grid, double x, double y)
{
    const auto cell = grid.worldToCell(x, y);
    return !grid.isOccupied(cell.row, cell.col);
}

bool trajectoryCollisionFree(const map::OccupancyGrid& grid,
                             double x0, double y0, double th0,
                             double steer, double wheelbase, double ds, int samples)
{
    double x = x0;
    double y = y0;
    double th = th0;
    const double step = ds / static_cast<double>(std::max(1, samples));
    for (int i = 0; i < samples; ++i) {
        // Bicycle: integrate path length step at unit speed.
        const double dth = (step / wheelbase) * std::tan(steer);
        if (std::abs(dth) < 1e-12) {
            x += step * std::cos(th);
            y += step * std::sin(th);
        } else {
            const double kappa = std::tan(steer) / wheelbase;
            x += (std::sin(th + dth) - std::sin(th)) / kappa;
            y += (-std::cos(th + dth) + std::cos(th)) / kappa;
            th = wrapToPi(th + dth);
        }
        if (!cellFree(grid, x, y)) {
            return false;
        }
    }
    return true;
}

bool pathCollisionFree(const map::OccupancyGrid& grid, const core::Path& path)
{
    for (const auto& w : path.waypoints()) {
        if (!cellFree(grid, w.x, w.y)) {
            return false;
        }
    }
    return true;
}

double heuristic(const SearchNode& n, const core::Pose& goal, double turning_radius)
{
    const auto segs = DubinsPath::shortestPath(
        core::Pose{n.x, n.y, n.theta}, goal, turning_radius);
    if (!segs.empty()) {
        return DubinsPath::length(segs);
    }
    return std::hypot(goal.x - n.x, goal.y - n.y);
}

}  // namespace

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

core::Path HybridAStarPlanner::plan(const map::OccupancyGrid& grid,
                                    const core::Pose& start,
                                    const core::Pose& goal) const
{
    if (wheelbase_m_ <= 0.0 || motion_resolution_m_ <= 0.0) {
        return {};
    }
    if (!cellFree(grid, start.x, start.y) || !cellFree(grid, goal.x, goal.y)) {
        return {};
    }

    const double r_min = minTurningRadiusM();
    const double xy_res = std::max(grid.resolutionM(), motion_resolution_m_ * 0.5);
    const double steers[] = {
        -max_steering_rad_,
        -0.5 * max_steering_rad_,
        0.0,
        0.5 * max_steering_rad_,
        max_steering_rad_
    };

    std::vector<SearchNode> nodes;
    nodes.reserve(4096);
    SearchNode root;
    root.x = start.x;
    root.y = start.y;
    root.theta = wrapToPi(start.theta);
    root.g = 0.0;
    root.f = heuristic(root, goal, r_min);
    root.parent = -1;
    nodes.push_back(root);

    std::priority_queue<OpenItem, std::vector<OpenItem>, std::greater<OpenItem>> open;
    open.push({root.f, 0});

    // best_g only (no permanent closed): same discrete key may reopen with better g.
    // Permanent closed was blocking feasible continuous states in narrow corridors.
    std::unordered_map<std::int64_t, double> best_g;

    const auto keyOf = [&](const SearchNode& n) {
        const int ix = static_cast<int>(std::floor(n.x / xy_res));
        const int iy = static_cast<int>(std::floor(n.y / xy_res));
        return stateKey(ix, iy, headingBin(n.theta));
    };
    best_g[keyOf(root)] = 0.0;

    int expansions = 0;
    int goal_index = -1;
    core::Path analytic_suffix;

    while (!open.empty() && expansions < kMaxExpansions) {
        const OpenItem item = open.top();
        open.pop();
        const int current_i = item.index;
        // Copy: later push_back may reallocate nodes and invalidate references.
        const SearchNode current = nodes[static_cast<std::size_t>(current_i)];

        const std::int64_t ckey = keyOf(current);
        const auto best_it = best_g.find(ckey);
        if (best_it != best_g.end() && current.g > best_it->second + 1e-9) {
            continue;  // stale
        }
        ++expansions;

        // Goal proximity (pose).
        const double dist_goal = std::hypot(goal.x - current.x, goal.y - current.y);
        const double dth = std::abs(wrapToPi(goal.theta - current.theta));
        if (dist_goal < std::max(0.35, 2.0 * grid.resolutionM()) && dth < 0.6) {
            goal_index = current_i;
            break;
        }

        // Analytic Dubins expansion.
        if (analytic_expand_every_n_ > 0
            && (expansions % analytic_expand_every_n_) == 0) {
            const auto segs = DubinsPath::shortestPath(
                core::Pose{current.x, current.y, current.theta}, goal, r_min);
            if (!segs.empty()) {
                const core::Path sampled = DubinsPath::sample(
                    core::Pose{current.x, current.y, current.theta},
                    segs, r_min, motion_resolution_m_);
                if (!sampled.empty() && pathCollisionFree(grid, sampled)) {
                    goal_index = current_i;
                    analytic_suffix = sampled;
                    break;
                }
            }
        }

        for (double steer : steers) {
            double x = current.x;
            double y = current.y;
            double th = current.theta;
            if (!trajectoryCollisionFree(grid, x, y, th, steer, wheelbase_m_,
                                         motion_resolution_m_, 4)) {
                continue;
            }
            // Apply same integration to get successor pose.
            {
                const double step = motion_resolution_m_;
                const double dth = (step / wheelbase_m_) * std::tan(steer);
                if (std::abs(dth) < 1e-12) {
                    x += step * std::cos(th);
                    y += step * std::sin(th);
                } else {
                    const double kappa = std::tan(steer) / wheelbase_m_;
                    x += (std::sin(th + dth) - std::sin(th)) / kappa;
                    y += (-std::cos(th + dth) + std::cos(th)) / kappa;
                    th = wrapToPi(th + dth);
                }
            }

            SearchNode child;
            child.x = x;
            child.y = y;
            child.theta = th;
            child.g = current.g + motion_resolution_m_;
            child.parent = current_i;
            child.f = child.g + heuristic(child, goal, r_min);

            const std::int64_t nkey = keyOf(child);
            const auto it = best_g.find(nkey);
            if (it != best_g.end() && child.g >= it->second - 1e-9) {
                continue;
            }
            best_g[nkey] = child.g;
            const int child_i = static_cast<int>(nodes.size());
            nodes.push_back(child);
            open.push({child.f, child_i});
        }
    }

    if (goal_index < 0) {
        return {};
    }

    std::vector<core::Waypoint> waypoints;
    for (int i = goal_index; i >= 0; i = nodes[static_cast<std::size_t>(i)].parent) {
        waypoints.push_back({nodes[static_cast<std::size_t>(i)].x,
                             nodes[static_cast<std::size_t>(i)].y});
        if (nodes[static_cast<std::size_t>(i)].parent == i) {
            break;
        }
    }
    std::reverse(waypoints.begin(), waypoints.end());

    if (!analytic_suffix.empty()) {
        const auto& suffix = analytic_suffix.waypoints();
        // Skip duplicate junction point.
        for (std::size_t i = 1; i < suffix.size(); ++i) {
            waypoints.push_back(suffix[i]);
        }
    } else {
        // Snap end toward goal XY.
        if (waypoints.empty()
            || std::hypot(waypoints.back().x - goal.x, waypoints.back().y - goal.y) > 0.05) {
            waypoints.push_back({goal.x, goal.y});
        }
    }

    return core::Path(std::move(waypoints));
}

}  // namespace fleetsim::domain::planning
