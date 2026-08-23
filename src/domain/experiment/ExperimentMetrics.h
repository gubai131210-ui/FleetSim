#pragma once

#include <cstddef>
#include <deque>

namespace fleetsim::domain::experiment {

struct TickSample {
    double cross_track_error{0.0};
    double heading_error{0.0};
    double linear_velocity{0.0};
    double st_ref_velocity{0.0};
    bool mpc_last_solve_ok{true};
};

struct RunSummary {
    double mean_abs_cross_track{0.0};
    double mean_abs_heading_error{0.0};
    double min_st_ref_velocity{0.0};
    double mpc_solve_rate{1.0};
    std::size_t sample_count{0};
};

/// Tick-level experiment metrics with RunSummary aggregation (ADR-016 / Phase 7-C).
class ExperimentMetrics {
public:
    void setMaxSamples(std::size_t max_samples);
    std::size_t maxSamples() const { return max_samples_; }

    void recordTick(const TickSample& sample);
    RunSummary summarize() const;
    void reset();

    std::size_t tickCount() const { return tick_count_; }

private:
    std::size_t max_samples_{500};
    std::size_t tick_count_{0};
    std::deque<TickSample> samples_;
};

}  // namespace fleetsim::domain::experiment
