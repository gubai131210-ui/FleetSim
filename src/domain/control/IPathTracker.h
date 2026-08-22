#pragma once

#include "core/types/ControlCommand.h"
#include "core/types/Path.h"
#include "core/types/Pose.h"

namespace fleetsim::domain::control {

class IPathTracker {
public:
    virtual ~IPathTracker() = default;

    virtual core::ControlCommand compute(const core::Pose& current_pose,
                                         const core::Path& reference_path,
                                         double dt) const = 0;
};

}  // namespace fleetsim::domain::control
