#pragma once

#include "core/types/SpeedProfile.h"
#include "domain/control/IPathTracker.h"

namespace fleetsim::domain::control {

/// Linear MPC lateral tracker stub (ADR-014). Session 0: honest empty control
/// (zero steer). Real prediction + Eigen dense QP lands in Session 1.
class MpcLateralTracker final : public IPathTracker {
public:
    explicit MpcLateralTracker(int horizon = 10,
                               double mpc_dt = 0.1,
                               double q_lat = 2.0,
                               double q_heading = 2.0,
                               double r_steer = 0.5,
                               double max_steering_rad = 0.6,
                               double wheelbase_m = 0.8,
                               double cruise_speed = 0.5);

    core::ControlCommand compute(const core::Pose& current_pose,
                                 const core::Path& reference_path,
                                 double dt) const override;

    /// Inject ST / cruise profile; nullptr clears. Index by nearest waypoint.
    void setSpeedProfile(const core::SpeedProfile* profile);

    bool lastSolveOk() const { return last_solve_ok_; }

    int horizon() const { return horizon_; }
    double mpcDt() const { return mpc_dt_; }
    double qLat() const { return q_lat_; }
    double qHeading() const { return q_heading_; }
    double rSteer() const { return r_steer_; }
    double maxSteeringRad() const { return max_steering_rad_; }
    double wheelbaseM() const { return wheelbase_m_; }
    double cruiseSpeed() const { return cruise_speed_; }

private:
    double resolveTargetSpeed(const core::Pose& pose,
                              const core::Path& path) const;

    int horizon_;
    double mpc_dt_;
    double q_lat_;
    double q_heading_;
    double r_steer_;
    double max_steering_rad_;
    double wheelbase_m_;
    double cruise_speed_;
    const core::SpeedProfile* speed_profile_{nullptr};
    mutable bool last_solve_ok_{false};
};

}  // namespace fleetsim::domain::control
