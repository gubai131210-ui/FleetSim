#pragma once

#include "core/types/Path.h"
#include "core/types/Pose.h"

namespace fleetsim::domain::prediction {

/// Extension point for peer trajectory prediction (ADR-016).
class IPeerPredictor {
public:
    virtual ~IPeerPredictor() = default;

    virtual core::Path predictPath(const core::Pose& current,
                                   double nominal_speed_mps,
                                   double horizon_s,
                                   double sample_dt_s) const = 0;
};

}  // namespace fleetsim::domain::prediction
