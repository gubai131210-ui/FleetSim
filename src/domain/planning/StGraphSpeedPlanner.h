#pragma once

#include "core/types/Path.h"
#include "core/types/SpeedProfile.h"

#include <vector>

namespace fleetsim::domain::planning {

/// Known peer along a geometric path (ADR-015). Session 3+ projects into ST.
struct PeerTrajectory {
    core::Path path;
    double nominal_speed{0.5};  // m/s along peer path
};

/// ST-Graph speed planner stub (ADR-015). Session 0: cruise-filled profile,
/// does NOT read peers into (s,t) obstacles. Real ST lands in Session 3–4.
class StGraphSpeedPlanner {
public:
    explicit StGraphSpeedPlanner(double v_max = 0.5,
                                 double a_max = 0.8,
                                 double dt_grid = 0.1);

    core::SpeedProfile plan(const core::Path& ego_path,
                            const std::vector<PeerTrajectory>& peers) const;

    double vMax() const { return v_max_; }
    double aMax() const { return a_max_; }
    double dtGrid() const { return dt_grid_; }

private:
    double v_max_;
    double a_max_;
    double dt_grid_;
};

}  // namespace fleetsim::domain::planning
