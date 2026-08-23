#include "domain/experiment/ExperimentMetrics.h"

#include <gtest/gtest.h>

#include <cmath>

using fleetsim::domain::experiment::ExperimentMetrics;
using fleetsim::domain::experiment::RunSummary;
using fleetsim::domain::experiment::TickSample;

TEST(ExperimentMetricsTest, RecordsSamplesAndAggregatesMeanAbsCrossTrack)
{
    ExperimentMetrics metrics;
    metrics.setMaxSamples(100);

    metrics.recordTick(TickSample{0.1, 0.05, 0.5, 0.45, true});
    metrics.recordTick(TickSample{-0.3, 0.1, 0.48, 0.40, true});
    metrics.recordTick(TickSample{0.2, -0.02, 0.52, 0.42, false});

    const RunSummary summary = metrics.summarize();
    EXPECT_EQ(summary.sample_count, 3u);
    EXPECT_NEAR(summary.mean_abs_cross_track, (0.1 + 0.3 + 0.2) / 3.0, 1e-9);
    EXPECT_NEAR(summary.mean_abs_heading_error, (0.05 + 0.1 + 0.02) / 3.0, 1e-9);
    EXPECT_NEAR(summary.mpc_solve_rate, 2.0 / 3.0, 1e-9);
}

TEST(ExperimentMetricsTest, RespectsMaxSamplesWindow)
{
    ExperimentMetrics metrics;
    metrics.setMaxSamples(2);

    metrics.recordTick(TickSample{1.0, 0.0, 0.5, 0.5, true});
    metrics.recordTick(TickSample{2.0, 0.0, 0.5, 0.5, true});
    metrics.recordTick(TickSample{3.0, 0.0, 0.5, 0.5, true});

    const RunSummary summary = metrics.summarize();
    EXPECT_EQ(summary.sample_count, 2u);
    EXPECT_NEAR(summary.mean_abs_cross_track, (2.0 + 3.0) / 2.0, 1e-9);
}

TEST(ExperimentMetricsTest, ResetClearsState)
{
    ExperimentMetrics metrics;
    metrics.recordTick(TickSample{0.5, 0.0, 0.5, 0.5, true});
    metrics.reset();
    EXPECT_EQ(metrics.tickCount(), 0u);
    EXPECT_EQ(metrics.summarize().sample_count, 0u);
}
