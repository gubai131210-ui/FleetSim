#include "domain/prediction/ConstantVelocityPredictor.h"

namespace fleetsim::domain::prediction {

core::Path ConstantVelocityPredictor::predictPath(const core::Pose& /*current*/,
                                                  double /*nominal_speed_mps*/,
                                                  double /*horizon_s*/,
                                                  double /*sample_dt_s*/) const
{
    // Session 0 stub — Session 1 implements CV extrapolation (ADR-016).
    return core::Path{};
}

}  // namespace fleetsim::domain::prediction
