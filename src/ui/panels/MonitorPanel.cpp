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
    cross_track_plot_->setMinimumHeight(160);
    layout->addWidget(cross_track_plot_);

    velocity_plot_ = new QCustomPlot(this);
    velocity_plot_->addGraph();
    velocity_plot_->graph(0)->setPen(QPen(Qt::blue));
    velocity_plot_->xAxis->setLabel(tr("Time (s)"));
    velocity_plot_->yAxis->setLabel(tr("Linear velocity (m/s)"));
    velocity_plot_->setMinimumHeight(160);
    layout->addWidget(velocity_plot_);
}

void MonitorPanel::appendSample(double sim_time_s,
                                double cross_track_error_m,
                                double linear_velocity_mps)
{
    time_axis_.append(sim_time_s);
    cross_track_series_.append(cross_track_error_m);
    velocity_series_.append(linear_velocity_mps);

    while (static_cast<std::size_t>(time_axis_.size()) > kMaxPoints) {
        time_axis_.removeFirst();
        cross_track_series_.removeFirst();
        velocity_series_.removeFirst();
    }
    replotCharts();
}

void MonitorPanel::clearSamples()
{
    time_axis_.clear();
    cross_track_series_.clear();
    velocity_series_.clear();
    replotCharts();
}

void MonitorPanel::replotCharts()
{
    cross_track_plot_->graph(0)->setData(time_axis_, cross_track_series_);
    cross_track_plot_->xAxis->setRange(time_axis_.isEmpty() ? 0.0 : time_axis_.first(),
                                       time_axis_.isEmpty() ? 1.0 : time_axis_.last());
    cross_track_plot_->yAxis->rescale(true);
    cross_track_plot_->replot(QCustomPlot::rpQueuedReplot);

    velocity_plot_->graph(0)->setData(time_axis_, velocity_series_);
    velocity_plot_->xAxis->setRange(time_axis_.isEmpty() ? 0.0 : time_axis_.first(),
                                    time_axis_.isEmpty() ? 1.0 : time_axis_.last());
    velocity_plot_->yAxis->rescale(true);
    velocity_plot_->replot(QCustomPlot::rpQueuedReplot);
}

}  // namespace fleetsim::ui
