#pragma once

#include "IPathTracker.h"

namespace fleetsim::domain::control {

class PurePursuitTracker : public IPathTracker {
public:
    PurePursuitTracker(double lookahead_m = 0.6,
                       double max_linear_velocity = 0.5,
                       double max_angular_velocity = 1.0,
                       double rotate_in_place_threshold_rad = 0.785,
                       double goal_tolerance_m = 0.2);

    core::ControlCommand compute(const core::Pose& current_pose,
                                 const core::Path& reference_path,
                                 double dt) const override;

private:
    static double normalizeAngle(double angle);
    static double distance(const core::Pose& a, const core::Waypoint& b);
    static core::Waypoint findLookaheadPoint(const core::Pose& pose,
                                             const core::Path& path,
                                             double lookahead_m);

    double lookahead_m_{0.6};
    double max_linear_velocity_{0.5};
    double max_angular_velocity_{1.0};
    double rotate_in_place_threshold_rad_{0.785};
    double goal_tolerance_m_{0.2};
};

}  // namespace fleetsim::domain::control
