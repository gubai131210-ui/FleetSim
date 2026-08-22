#pragma once

#include "core/types/Path.h"
#include "core/types/Pose.h"
#include "domain/map/OccupancyGrid.h"

namespace fleetsim::domain::planning {

class IPathPlanner {
public:
    virtual ~IPathPlanner() = default;

    virtual core::Path plan(const map::OccupancyGrid& grid,
                            const core::Pose& start,
                            const core::Pose& goal) const = 0;
};

}  // namespace fleetsim::domain::planning
