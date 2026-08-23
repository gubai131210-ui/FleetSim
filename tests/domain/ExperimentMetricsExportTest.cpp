#include "domain/experiment/ExperimentMetrics.h"

#include <gtest/gtest.h>

#include <fstream>
#include <filesystem>
#include <sstream>
#include <string>

using fleetsim::domain::experiment::ExperimentMetrics;
using fleetsim::domain::experiment::TickSample;

namespace {

std::string readFile(const std::string& path)
{
    std::ifstream input(path);
    std::stringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

std::size_t countLines(const std::string& text)
{
    if (text.empty()) {
        return 0;
    }
    std::size_t lines = 0;
    for (char ch : text) {
        if (ch == '\n') {
            ++lines;
        }
    }
    if (text.back() != '\n') {
        ++lines;
    }
    return lines;
}

}  // namespace

TEST(ExperimentMetricsExportTest, ExportCsvWritesHeaderAndSampleRows)
{
    std::filesystem::create_directories("test_tmp");
    ExperimentMetrics metrics;
    metrics.setMaxSamples(100);
    metrics.recordTick(TickSample{0.1, 0.05, 0.5, 0.45, true});
    metrics.recordTick(TickSample{-0.2, 0.02, 0.48, 0.40, false});
    metrics.recordTick(TickSample{0.0, 0.0, 0.52, 0.42, true});

    const std::string path = "test_tmp/experiment_metrics_export.csv";
    ASSERT_TRUE(metrics.exportCsv(path));

    const std::string csv = readFile(path);
    EXPECT_NE(csv.find("tick,cross_track,heading_error,st_ref_v,mpc_ok"), std::string::npos);
    EXPECT_EQ(countLines(csv), 4u);
    EXPECT_EQ(metrics.summarize().sample_count, 3u);
}

TEST(ExperimentMetricsExportTest, ExportCsvLineCountMatchesSampleCount)
{
    ExperimentMetrics metrics;
    for (int i = 0; i < 7; ++i) {
        metrics.recordTick(TickSample{static_cast<double>(i), 0.0, 0.5, 0.5, true});
    }

    const std::string path = "test_tmp/experiment_metrics_export_count.csv";
    ASSERT_TRUE(metrics.exportCsv(path));
    EXPECT_EQ(countLines(readFile(path)), 8u);
    EXPECT_EQ(metrics.storedSampleCount(), 7u);
}

TEST(ExperimentMetricsExportTest, ExportEmptyMetricsWritesHeaderOnly)
{
    ExperimentMetrics metrics;
    const std::string path = "test_tmp/experiment_metrics_export_empty.csv";
    ASSERT_TRUE(metrics.exportCsv(path));
    EXPECT_EQ(countLines(readFile(path)), 1u);
}
