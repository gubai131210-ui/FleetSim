#pragma once

#include "core/types/Path.h"

namespace fleetsim::domain::planning {

class IPathSmoother {
public:
    virtual ~IPathSmoother() = default;

    virtual core::Path smooth(const core::Path& raw_path) const = 0;
};

}  // namespace fleetsim::domain::planning
