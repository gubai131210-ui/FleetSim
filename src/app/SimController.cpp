#include "SimController.h"

namespace fleetsim::app {

SimController::SimController() = default;

domain::SimEngine& SimController::engine()
{
    return engine_;
}

const domain::SimEngine& SimController::engine() const
{
    return engine_;
}

void SimController::start()
{
    running_ = true;
    engine_.clock().resume();
}

void SimController::pause()
{
    running_ = false;
    engine_.clock().pause();
}

void SimController::stepOnce()
{
    const double dt = engine_.clock().fixedDt();
    engine_.clock().tick();
    engine_.tick(dt);
}

void SimController::tick()
{
    if (!running_) {
        return;
    }

    const double dt = engine_.clock().tick();
    if (dt > 0.0) {
        engine_.tick(dt);
    }
}

bool SimController::isRunning() const
{
    return running_;
}

}  // namespace fleetsim::app
