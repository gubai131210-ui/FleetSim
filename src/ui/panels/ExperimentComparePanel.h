#pragma once

#include <QWidget>

class QLabel;
class QPushButton;
class QTableWidget;

namespace fleetsim::ui {

/// Aggregated experiment metrics vs baseline (ADR-017). Not merged into MonitorPanel.
class ExperimentComparePanel : public QWidget {
    Q_OBJECT

public:
    explicit ExperimentComparePanel(QWidget* parent = nullptr);

public slots:
    void updateCurrentRun(double mean_abs_cross_track,
                          double mean_abs_heading_error,
                          double min_st_ref_velocity,
                          double mpc_solve_rate,
                          quint64 sample_count);
    void updateBaseline(double mean_abs_cross_track,
                        double mean_abs_heading_error,
                        double min_st_ref_velocity,
                        double mpc_solve_rate,
                        quint64 sample_count);
    void clearBaseline();

signals:
    void captureBaselineRequested();
    void exportCsvRequested(const QString& path);

private:
    void setRow(int row,
                const QString& label,
                double current,
                double baseline,
                bool has_baseline,
                int precision = 3);

    QTableWidget* table_{nullptr};
    QLabel* status_label_{nullptr};
    QPushButton* capture_button_{nullptr};
    bool has_baseline_{false};
    double current_cross_track_{0.0};
    double current_heading_{0.0};
    double current_min_st_{0.0};
    double current_mpc_rate_{0.0};
    double baseline_cross_track_{0.0};
    double baseline_heading_{0.0};
    double baseline_min_st_{0.0};
    double baseline_mpc_rate_{0.0};
};

}  // namespace fleetsim::ui
