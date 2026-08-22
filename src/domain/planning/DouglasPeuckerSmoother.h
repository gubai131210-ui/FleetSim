#pragma once

#include "IPathSmoother.h"

namespace fleetsim::domain::planning {

class DouglasPeuckerSmoother : public IPathSmoother {
public:
    explicit DouglasPeuckerSmoother(double epsilon_m = 0.15,
                                    double interpolate_step_m = 0.2);

    core::Path smooth(const core::Path& raw_path) const override;

private:
    static double perpendicularDistance(const core::Waypoint& point,
                                        const core::Waypoint& line_start,
                                        const core::Waypoint& line_end);

    static void douglasPeucker(const std::vector<core::Waypoint>& points,
                               std::size_t start_index,
                               std::size_t end_index,
                               double epsilon,
                               std::vector<bool>& keep) ;

    core::Path interpolate(const std::vector<core::Waypoint>& simplified) const;

    double epsilon_m_{0.15};
    double interpolate_step_m_{0.2};
};

}  // namespace fleetsim::domain::planning
