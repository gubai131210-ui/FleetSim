#pragma once

#include "prediction/IPeerPredictor.h"

namespace fleetsim::domain::prediction {

/// Constant-velocity extrapolation along current heading (ADR-016).
class ConstantVelocityPredictor : public IPeerPredictor {
public:
    core::Path predictPath(const core::Pose& current,
                           double nominal_speed_mps,
                           double horizon_s,
                           double sample_dt_s) const override;
};

}  // namespace fleetsim::domain::prediction
