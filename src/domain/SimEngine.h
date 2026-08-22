#pragma once

#include "scheduling/SchedulingModule.h"
#include "collision/CollisionModule.h"

#include "core/EventBus.h"
#include "core/SimClock.h"

namespace fleetsim::domain {

class SimEngine {
public:
    SimEngine();

    core::SimClock& clock();
    const core::SimClock& clock() const;

    core::EventBus& eventBus();
    const core::EventBus& eventBus() const;

    scheduling::SchedulingModule& scheduling();
    const scheduling::SchedulingModule& scheduling() const;

    collision::CollisionModule& collision();
    const collision::CollisionModule& collision() const;

    void tick(double dt);

    int tickCount() const;

private:
    core::SimClock clock_;
    core::EventBus event_bus_;
    scheduling::SchedulingModule scheduling_;
    collision::CollisionModule collision_;
    int tick_count_{0};
};

}  // namespace fleetsim::domain
