#pragma once

#include <cmath>

namespace fleetsim::core {

struct Pose {
    double x{0.0};
    double y{0.0};
    double theta{0.0};

    bool operator==(const Pose& other) const
    {
        constexpr double kEpsilon = 1e-9;
        return std::abs(x - other.x) < kEpsilon
            && std::abs(y - other.y) < kEpsilon
            && std::abs(theta - other.theta) < kEpsilon;
    }
};

}  // namespace fleetsim::core
