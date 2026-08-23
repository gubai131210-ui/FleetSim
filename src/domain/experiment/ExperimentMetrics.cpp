#include "domain/experiment/ExperimentMetrics.h"

#include <algorithm>
#include <cmath>
#include <deque>
#include <fstream>
#include <limits>

namespace fleetsim::domain::experiment {

void ExperimentMetrics::setMaxSamples(std::size_t max_samples)
{
    max_samples_ = std::max(max_samples, std::size_t{1});
    while (samples_.size() > max_samples_) {
        samples_.pop_front();
    }
}

void ExperimentMetrics::recordTick(const TickSample& sample)
{
    ++tick_count_;
    samples_.push_back(sample);
    while (samples_.size() > max_samples_) {
        samples_.pop_front();
    }
}

RunSummary ExperimentMetrics::summarize() const
{
    RunSummary summary;
    summary.sample_count = samples_.size();
    if (samples_.empty()) {
        return summary;
    }

    double sum_abs_cross_track = 0.0;
    double sum_abs_heading = 0.0;
    double min_st_ref = std::numeric_limits<double>::infinity();
    std::size_t mpc_ok = 0;

    for (const TickSample& sample : samples_) {
        sum_abs_cross_track += std::abs(sample.cross_track_error);
        sum_abs_heading += std::abs(sample.heading_error);
        min_st_ref = std::min(min_st_ref, sample.st_ref_velocity);
        if (sample.mpc_last_solve_ok) {
            ++mpc_ok;
        }
    }

    const double n = static_cast<double>(samples_.size());
    summary.mean_abs_cross_track = sum_abs_cross_track / n;
    summary.mean_abs_heading_error = sum_abs_heading / n;
    summary.min_st_ref_velocity = min_st_ref;
    summary.mpc_solve_rate = static_cast<double>(mpc_ok) / n;
    return summary;
}

void ExperimentMetrics::reset()
{
    tick_count_ = 0;
    samples_.clear();
}

bool ExperimentMetrics::exportCsv(const std::string& path) const
{
    std::ofstream output(path);
    if (!output.is_open()) {
        return false;
    }

    output << "tick,cross_track,heading_error,st_ref_v,mpc_ok\n";
    std::size_t tick_index = 0;
    for (const TickSample& sample : samples_) {
        ++tick_index;
        output << tick_index << ','
               << sample.cross_track_error << ','
               << sample.heading_error << ','
               << sample.st_ref_velocity << ','
               << (sample.mpc_last_solve_ok ? 1 : 0) << '\n';
    }
    return output.good();
}

}  // namespace fleetsim::domain::experiment
