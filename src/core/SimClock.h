#pragma once

namespace fleetsim::core {

enum class SimClockMode {
    FixedStep,
    RealTime
};

class SimClock {
public:
    explicit SimClock(double fixed_dt_s = 0.05);

    void setMode(SimClockMode mode);
    SimClockMode mode() const;

    void setFixedDt(double dt_s);
    double fixedDt() const;

    void setTimeScale(double scale);
    double timeScale() const;

    void pause();
    void resume();
    bool isPaused() const;

    /// Advances simulation time and returns the dt applied this tick.
    double tick();

    double simTime() const;
    int tickCount() const;

    void reset();

private:
    SimClockMode mode_{SimClockMode::FixedStep};
    double fixed_dt_s_{0.05};
    double time_scale_{1.0};
    double sim_time_s_{0.0};
    int tick_count_{0};
    bool paused_{false};
};

}  // namespace fleetsim::core
