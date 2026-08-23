#pragma once

#include "core/types/Path.h"
#include "core/types/SpeedProfile.h"

#include <vector>

namespace fleetsim::domain::planning {

/// Known peer along a geometric path (ADR-015; ADR-016 from_prediction).
struct PeerTrajectory {
    core::Path path;
    double nominal_speed{0.5};  // m/s along peer path
    bool from_prediction{false};
};

/// ST-Graph speed planner (ADR-015): project peers into (s,t) blocks and
/// produce an equal-length SpeedProfile. Not Euclidean distance-stop.
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
    struct Occupancy {
        double s_min{0.0};
        double s_max{0.0};
        double t_min{0.0};
        double t_max{0.0};
    };

    static std::vector<double> cumulativeArcLength(const core::Path& path);
    static bool projectOntoPath(const core::Path& path,
                                double x,
                                double y,
                                double* s_out,
                                double* lateral_out);
    std::vector<Occupancy> buildOccupancies(
        const core::Path& ego_path,
        const std::vector<double>& ego_s,
        const std::vector<PeerTrajectory>& peers) const;

    double v_max_;
    double a_max_;
    double dt_grid_;
};

}  // namespace fleetsim::domain::planning
