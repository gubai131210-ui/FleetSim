#pragma once

#include "IPathPlanner.h"

namespace fleetsim::domain::planning {

class AStarPlanner : public IPathPlanner {
public:
    core::Path plan(const map::OccupancyGrid& grid,
                    const core::Pose& start,
                    const core::Pose& goal) const override;

private:
    static double octileHeuristic(int row0, int col0, int row1, int col1, double resolution_m);
};

}  // namespace fleetsim::domain::planning
