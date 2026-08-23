#include "domain/prediction/ConstantVelocityPredictor.h"

#include "core/types/Waypoint.h"

#include <cmath>
#include <vector>

namespace fleetsim::domain::prediction {

core::Path ConstantVelocityPredictor::predictPath(const core::Pose& current,
                                                  double nominal_speed_mps,
                                                  double horizon_s,
                                                  double sample_dt_s) const
{
    std::vector<core::Waypoint> waypoints;

    if (sample_dt_s <= 0.0) {
        waypoints.push_back({current.x, current.y});
        return core::Path(std::move(waypoints));
    }

    const int steps = horizon_s <= 0.0
                          ? 0
                          : static_cast<int>(std::floor(horizon_s / sample_dt_s));

    waypoints.reserve(static_cast<std::size_t>(steps) + 1u);

    const double cos_theta = std::cos(current.theta);
    const double sin_theta = std::sin(current.theta);

    for (int k = 0; k <= steps; ++k) {
        const double elapsed_s = static_cast<double>(k) * sample_dt_s;
        waypoints.push_back({
            current.x + nominal_speed_mps * cos_theta * elapsed_s,
            current.y + nominal_speed_mps * sin_theta * elapsed_s,
        });
    }

    return core::Path(std::move(waypoints));
}

}  // namespace fleetsim::domain::prediction
