#pragma once

namespace fleetsim::core {

struct Waypoint {
    double x{0.0};
    double y{0.0};

    bool operator==(const Waypoint& other) const
    {
        return x == other.x && y == other.y;
    }
};

}  // namespace fleetsim::core
