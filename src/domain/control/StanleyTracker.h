#pragma once

#include "domain/control/IPathTracker.h"

namespace fleetsim::domain::control {

/// Stanley tracker: front-axle cross-track + heading error (ADR-012).
/// δ = θe + arctan(k e / (v+ε)); not a Pure Pursuit skin.
class StanleyTracker final : public IPathTracker {
public:
    explicit StanleyTracker(double k_gain = 1.5,
                            double softening_v = 0.1,
                            double max_steering_rad = 0.6,
                            double wheelbase_m = 0.8,
                            double cruise_velocity = 0.5);

    core::ControlCommand compute(const core::Pose& current_pose,
                                 const core::Path& reference_path,
                                 double dt) const override;

    double kGain() const { return k_gain_; }
    double softeningV() const { return softening_v_; }
    double maxSteeringRad() const { return max_steering_rad_; }
    double wheelbaseM() const { return wheelbase_m_; }

private:
    double k_gain_;
    double softening_v_;
    double max_steering_rad_;
    double wheelbase_m_;
    double cruise_velocity_;
};

}  // namespace fleetsim::domain::control
