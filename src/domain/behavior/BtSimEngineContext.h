#pragma once

#include "IBtSimContext.h"

#include "core/types/Pose.h"
#include "core/types/VehicleId.h"

namespace fleetsim::domain {
class SimEngine;
}

namespace fleetsim::domain::vehicle {
struct VehicleAgent;
}

namespace fleetsim::domain::behavior {

/// Adapts SimEngine + agent id for BT leaf ticks (Session 2+).
class BtSimEngineContext final : public IBtSimContext {
public:
    BtSimEngineContext(SimEngine& engine, core::VehicleId agent_id);

    bool planPathForAgent() override;
    bool hasValidPath() const override;
    bool isGoalReached() const override;
    bool isGoalUpdated() override;
    void acknowledgeGoal() override;
    bool hasReservationConflict() const override;
    void requestReplan() override;
    bool needsReplan() const override;
    void clearReplanRequest() override;
    double simDt() const override;

    core::Pose agentPose() const override;
    void applyYawDelta(double delta_rad) override;
    void applyBodyTranslation(double forward_m, double lateral_m) override;
    bool clearInflationLayer() override;
    std::size_t occupiedCellCount() const override;

    SimEngine& engine() { return engine_; }
    const core::VehicleId& agentId() const { return agent_id_; }

private:
    const vehicle::VehicleAgent* agent() const;
    vehicle::VehicleAgent* agent();

    SimEngine& engine_;
    core::VehicleId agent_id_;
    core::Pose last_goal_{};
    bool goal_initialized_{false};
};

}  // namespace fleetsim::domain::behavior
