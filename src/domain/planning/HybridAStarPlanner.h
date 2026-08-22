#pragma once

#include "domain/planning/IPathPlanner.h"

namespace fleetsim::domain::planning {

/// Hybrid A* planner: continuous state (x,y,θ) with bicycle steering expansion
/// and periodic Dubins analytic connection (ADR-011). Forward-only MVP.
class HybridAStarPlanner final : public IPathPlanner {
public:
    explicit HybridAStarPlanner(double wheelbase_m = 0.8,
                                double max_steering_rad = 0.6,
                                double motion_resolution_m = 0.25,
                                int analytic_expand_every_n = 5);

    core::Path plan(const map::OccupancyGrid& grid,
                    const core::Pose& start,
                    const core::Pose& goal) const override;

    double wheelbaseM() const { return wheelbase_m_; }
    double maxSteeringRad() const { return max_steering_rad_; }
    double minTurningRadiusM() const;

private:
    double wheelbase_m_;
    double max_steering_rad_;
    double motion_resolution_m_;
    int analytic_expand_every_n_;
};

}  // namespace fleetsim::domain::planning
