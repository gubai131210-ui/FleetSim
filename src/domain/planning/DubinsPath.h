#pragma once

#include "core/types/Path.h"
#include "core/types/Pose.h"

#include <vector>

namespace fleetsim::domain::planning {

/// Forward-only Dubins curves (CSC + CCC). Used by Hybrid A* analytic expansion (ADR-011).
class DubinsPath {
public:
    enum class SegmentType { kLeft, kStraight, kRight };

    struct Segment {
        SegmentType type{SegmentType::kStraight};
        double length{0.0};  // arc length or straight length (meters); turning radius scaled
    };

    /// Compute shortest Dubins path at given turning radius. Returns empty on failure.
    static std::vector<Segment> shortestPath(const core::Pose& start,
                                             const core::Pose& goal,
                                             double turning_radius);

    /// Sample waypoints along segments at approximately step_m spacing (includes endpoints).
    static core::Path sample(const core::Pose& start,
                             const std::vector<Segment>& segments,
                             double turning_radius,
                             double step_m);

    /// Path length in meters (sum of segment lengths).
    static double length(const std::vector<Segment>& segments);
};

}  // namespace fleetsim::domain::planning
