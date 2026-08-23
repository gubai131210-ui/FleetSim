#include "domain/experiment/ExperimentMetrics.h"

namespace fleetsim::domain::experiment {

void ExperimentMetrics::setMaxSamples(std::size_t max_samples)
{
    max_samples_ = max_samples;
}

void ExperimentMetrics::recordTick(const TickSample& /*sample*/)
{
    // Session 0 stub — Session 3 implements rolling window + aggregation.
    ++tick_count_;
}

RunSummary ExperimentMetrics::summarize() const
{
    RunSummary summary;
    summary.sample_count = tick_count_;
    return summary;
}

void ExperimentMetrics::reset()
{
    tick_count_ = 0;
}

}  // namespace fleetsim::domain::experiment
