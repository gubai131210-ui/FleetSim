#include "MonitorPanel.h"

#include <QVBoxLayout>

namespace fleetsim::ui {

MonitorPanel::MonitorPanel(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);

    cross_track_plot_ = new QCustomPlot(this);
    cross_track_plot_->addGraph();
    cross_track_plot_->graph(0)->setPen(QPen(Qt::red));
    cross_track_plot_->xAxis->setLabel(tr("Time (s)"));
    cross_track_plot_->yAxis->setLabel(tr("Cross-track error (m)"));
    cross_track_plot_->setMinimumHeight(120);
    layout->addWidget(cross_track_plot_);

    heading_plot_ = new QCustomPlot(this);
    heading_plot_->addGraph();
    heading_plot_->graph(0)->setPen(QPen(QColor(180, 90, 0)));
    heading_plot_->xAxis->setLabel(tr("Time (s)"));
    heading_plot_->yAxis->setLabel(tr("Heading error (rad)"));
    heading_plot_->setMinimumHeight(120);
    layout->addWidget(heading_plot_);

    // Graph 0: actual linear velocity; graph 1: ST reference speed (Phase 6).
    velocity_plot_ = new QCustomPlot(this);
    velocity_plot_->addGraph();
    velocity_plot_->graph(0)->setPen(QPen(Qt::blue));
    velocity_plot_->graph(0)->setName(tr("Actual v"));
    velocity_plot_->addGraph();
    velocity_plot_->graph(1)->setPen(QPen(QColor(0, 140, 70)));
    velocity_plot_->graph(1)->setName(tr("ST ref v"));
    velocity_plot_->legend->setVisible(true);
    velocity_plot_->xAxis->setLabel(tr("Time (s)"));
    velocity_plot_->yAxis->setLabel(tr("Linear velocity (m/s)"));
    velocity_plot_->setMinimumHeight(140);
    layout->addWidget(velocity_plot_);
}

void MonitorPanel::appendSample(double sim_time_s,
                                double cross_track_error_m,
                                double heading_error_rad,
                                double linear_velocity_mps,
                                double st_reference_speed_mps)
{
    time_axis_.append(sim_time_s);
    cross_track_series_.append(cross_track_error_m);
    heading_series_.append(heading_error_rad);
    velocity_series_.append(linear_velocity_mps);
    st_speed_series_.append(st_reference_speed_mps);

    while (static_cast<std::size_t>(time_axis_.size()) > kMaxPoints) {
        time_axis_.removeFirst();
        cross_track_series_.removeFirst();
        heading_series_.removeFirst();
        velocity_series_.removeFirst();
        st_speed_series_.removeFirst();
    }
    replotCharts();
}

void MonitorPanel::clearSamples()
{
    time_axis_.clear();
    cross_track_series_.clear();
    heading_series_.clear();
    velocity_series_.clear();
    st_speed_series_.clear();
    replotCharts();
}

void MonitorPanel::replotCharts()
{
    const double t0 = time_axis_.isEmpty() ? 0.0 : time_axis_.first();
    const double t1 = time_axis_.isEmpty() ? 1.0 : time_axis_.last();

    cross_track_plot_->graph(0)->setData(time_axis_, cross_track_series_);
    cross_track_plot_->xAxis->setRange(t0, t1);
    cross_track_plot_->yAxis->rescale(true);
    cross_track_plot_->replot(QCustomPlot::rpQueuedReplot);

    heading_plot_->graph(0)->setData(time_axis_, heading_series_);
    heading_plot_->xAxis->setRange(t0, t1);
    heading_plot_->yAxis->rescale(true);
    heading_plot_->replot(QCustomPlot::rpQueuedReplot);

    velocity_plot_->graph(0)->setData(time_axis_, velocity_series_);
    velocity_plot_->graph(1)->setData(time_axis_, st_speed_series_);
    velocity_plot_->xAxis->setRange(t0, t1);
    velocity_plot_->yAxis->rescale(true);
    velocity_plot_->replot(QCustomPlot::rpQueuedReplot);
}

}  // namespace fleetsim::ui
