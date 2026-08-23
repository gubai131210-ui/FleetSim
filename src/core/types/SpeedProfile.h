#pragma once

#include <vector>

namespace fleetsim::core {

/// Longitudinal speed / timing profile along a Path (ADR-015).
/// Contract: speeds.size() == arrival_times.size() == path.waypoints().size()
/// (empty path → both vectors empty). Not a Waypoint field extension.
struct SpeedProfile {
    std::vector<double> speeds;         // m/s at each waypoint
    std::vector<double> arrival_times;  // s from path start; arrival_times[0] == 0
};

}  // namespace fleetsim::core
