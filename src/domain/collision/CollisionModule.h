#pragma once

namespace fleetsim::domain::collision {

/// Phase 3: time-window reservation + priority yielding.
/// Phase 0–2: no-op stub registered with SimEngine.
class CollisionModule {
public:
    void tick(double dt);

    int conflictCount() const;

private:
    int conflicts_{0};
};

}  // namespace fleetsim::domain::collision
