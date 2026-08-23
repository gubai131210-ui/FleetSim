#pragma once

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
};

}  // namespace fleetsim::domain::behavior
