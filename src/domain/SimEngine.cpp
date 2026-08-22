#include "SimEngine.h"

namespace fleetsim::domain {

SimEngine::SimEngine() = default;

core::SimClock& SimEngine::clock()
{
    return clock_;
}

const core::SimClock& SimEngine::clock() const
{
    return clock_;
}

core::EventBus& SimEngine::eventBus()
{
    return event_bus_;
}

const core::EventBus& SimEngine::eventBus() const
{
    return event_bus_;
}

scheduling::SchedulingModule& SimEngine::scheduling()
{
    return scheduling_;
}

const scheduling::SchedulingModule& SimEngine::scheduling() const
{
    return scheduling_;
}

collision::CollisionModule& SimEngine::collision()
{
    return collision_;
}

const collision::CollisionModule& SimEngine::collision() const
{
    return collision_;
}

void SimEngine::tick(double dt)
{
    if (clock_.isPaused()) {
        return;
    }

    scheduling_.tick(dt);
    collision_.tick(dt);

    ++tick_count_;
    event_bus_.publish("sim/tick", std::to_string(tick_count_));
}

int SimEngine::tickCount() const
{
    return tick_count_;
}

}  // namespace fleetsim::domain
