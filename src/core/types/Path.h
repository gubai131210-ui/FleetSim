#pragma once

#include "Waypoint.h"

#include <cmath>
#include <vector>

namespace fleetsim::core {

class Path {
public:
    Path() = default;

    explicit Path(std::vector<Waypoint> waypoints)
        : waypoints_(std::move(waypoints))
    {
    }

    const std::vector<Waypoint>& waypoints() const { return waypoints_; }
    std::vector<Waypoint>& waypoints() { return waypoints_; }

    bool empty() const { return waypoints_.empty(); }
    std::size_t size() const { return waypoints_.size(); }

    void clear() { waypoints_.clear(); }

    double length() const
    {
        if (waypoints_.size() < 2) {
            return 0.0;
        }

        double total = 0.0;
        for (std::size_t i = 1; i < waypoints_.size(); ++i) {
            const double dx = waypoints_[i].x - waypoints_[i - 1].x;
            const double dy = waypoints_[i].y - waypoints_[i - 1].y;
            total += std::sqrt(dx * dx + dy * dy);
        }
        return total;
    }

private:
    std::vector<Waypoint> waypoints_;
};

}  // namespace fleetsim::core
