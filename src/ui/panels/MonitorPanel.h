#pragma once

#include "qcustomplot.h"

#include <QWidget>

#include <deque>

namespace fleetsim::ui {

class MonitorPanel : public QWidget {
    Q_OBJECT

public:
    explicit MonitorPanel(QWidget* parent = nullptr);

public slots:
    void appendSample(double sim_time_s, double cross_track_error_m, double linear_velocity_mps);
    void clearSamples();

private:
    void replotCharts();

    QCustomPlot* cross_track_plot_{nullptr};
    QCustomPlot* velocity_plot_{nullptr};
    QVector<double> time_axis_;
    QVector<double> cross_track_series_;
    QVector<double> velocity_series_;
    static constexpr std::size_t kMaxPoints = 500;
};

}  // namespace fleetsim::ui
