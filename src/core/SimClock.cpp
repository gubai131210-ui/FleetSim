#include "SimClock.h"

namespace fleetsim::core {

SimClock::SimClock(double fixed_dt_s)
    : fixed_dt_s_(fixed_dt_s)
{
}

void SimClock::setMode(SimClockMode mode)
{
    mode_ = mode;
}

SimClockMode SimClock::mode() const
{
    return mode_;
}

void SimClock::setFixedDt(double dt_s)
{
    fixed_dt_s_ = dt_s;
}

double SimClock::fixedDt() const
{
    return fixed_dt_s_;
}

void SimClock::setTimeScale(double scale)
{
    time_scale_ = scale;
}

double SimClock::timeScale() const
{
    return time_scale_;
}

void SimClock::pause()
{
    paused_ = true;
}

void SimClock::resume()
{
    paused_ = false;
}

bool SimClock::isPaused() const
{
    return paused_;
}

double SimClock::tick()
{
    if (paused_) {
        return 0.0;
    }

    const double dt = fixed_dt_s_ * time_scale_;
    sim_time_s_ += dt;
    ++tick_count_;
    return dt;
}

double SimClock::simTime() const
{
    return sim_time_s_;
}

int SimClock::tickCount() const
{
    return tick_count_;
}

void SimClock::reset()
{
    sim_time_s_ = 0.0;
    tick_count_ = 0;
    paused_ = false;
}

}  // namespace fleetsim::core
