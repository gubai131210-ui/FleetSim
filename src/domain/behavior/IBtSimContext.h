#pragma once

#include "core/types/Pose.h"

#include <cstddef>

namespace fleetsim::domain::behavior {

/// Simulation bridge for BT leaf nodes (zero Qt; implemented by SimEngine adapter).
class IBtSimContext {
public:
    virtual ~IBtSimContext() = default;

    virtual bool planPathForAgent() = 0;
    virtual bool hasValidPath() const = 0;
    virtual bool isGoalReached() const = 0;
    virtual bool isGoalUpdated() = 0;
    virtual void acknowledgeGoal() = 0;
    virtual bool hasReservationConflict() const = 0;
    virtual void requestReplan() = 0;
    virtual bool needsReplan() const = 0;
    virtual void clearReplanRequest() = 0;
    virtual double simDt() const = 0;

    /// Motion recovery hooks (ADR-023). Defaults are no-ops for legacy mocks.
    virtual core::Pose agentPose() const { return core::Pose{}; }
    virtual void applyYawDelta(double /*delta_rad*/) {}
    virtual void applyBodyTranslation(double /*forward_m*/, double /*lateral_m*/) {}
    virtual bool clearInflationLayer() { return false; }
    virtual std::size_t occupiedCellCount() const { return 0U; }
};

}  // namespace fleetsim::domain::behavior
