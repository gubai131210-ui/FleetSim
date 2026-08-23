#include "domain/planning/StGraphSpeedPlanner.h"

#include <cmath>

namespace fleetsim::domain::planning {

StGraphSpeedPlanner::StGraphSpeedPlanner(double v_max, double a_max, double dt_grid)
    : v_max_(v_max)
    , a_max_(a_max)
    , dt_grid_(dt_grid)
{
}

core::SpeedProfile StGraphSpeedPlanner::plan(const core::Path& ego_path,
                                             const std::vector<PeerTrajectory>& /*peers*/) const
{
    // Session 0 stub: equal-length cruise profile; peers intentionally ignored.
    // Session 3 fills (s,t) obstacles from peers; Session 4 wires SimEngine.
    core::SpeedProfile profile;
    if (ego_path.empty()) {
        return profile;
    }

    const std::size_t n = ego_path.size();
    profile.speeds.assign(n, v_max_);
    profile.arrival_times.assign(n, 0.0);

    const auto& w = ego_path.waypoints();
    double t = 0.0;
    profile.arrival_times[0] = 0.0;
    for (std::size_t i = 1; i < n; ++i) {
        const double dx = w[i].x - w[i - 1].x;
        const double dy = w[i].y - w[i - 1].y;
        const double ds = std::sqrt(dx * dx + dy * dy);
        const double v = (v_max_ > 1e-9) ? v_max_ : 1e-9;
        t += ds / v;
        profile.arrival_times[i] = t;
    }
    return profile;
}

}  // namespace fleetsim::domain::planning
