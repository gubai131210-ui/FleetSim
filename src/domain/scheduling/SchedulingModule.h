#pragma once

namespace fleetsim::domain::scheduling {

/// Phase 3: task queue and ITaskAssigner implementations.
/// Phase 0–2: no-op stub registered with SimEngine.
class SchedulingModule {
public:
    void tick(double dt);

    int pendingTaskCount() const;

private:
    int pending_tasks_{0};
};

}  // namespace fleetsim::domain::scheduling
