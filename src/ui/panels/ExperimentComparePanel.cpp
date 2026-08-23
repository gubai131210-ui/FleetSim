#include "ExperimentComparePanel.h"

#include <QFileDialog>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

namespace fleetsim::ui {

ExperimentComparePanel::ExperimentComparePanel(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(
        tr("Compare aggregated run metrics against a captured baseline."), this));

    table_ = new QTableWidget(4, 3, this);
    table_->setHorizontalHeaderLabels({tr("Metric"), tr("Current"), tr("Baseline")});
    table_->verticalHeader()->setVisible(false);
    table_->horizontalHeader()->setStretchLastSection(true);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setSelectionMode(QAbstractItemView::NoSelection);
    layout->addWidget(table_);

    capture_button_ = new QPushButton(tr("Capture Baseline"), this);
    connect(capture_button_, &QPushButton::clicked, this, [this]() {
        emit captureBaselineRequested();
    });
    layout->addWidget(capture_button_);

    auto* export_button = new QPushButton(tr("Export CSV"), this);
    connect(export_button, &QPushButton::clicked, this, [this]() {
        const QString path = QFileDialog::getSaveFileName(
            this, tr("Export experiment metrics CSV"), QString(), tr("CSV (*.csv)"));
        if (!path.isEmpty()) {
            emit exportCsvRequested(path);
        }
    });
    layout->addWidget(export_button);

    status_label_ = new QLabel(tr("Baseline: not captured"), this);
    layout->addWidget(status_label_);

    setRow(0, tr("Mean |cross-track| (m)"), 0.0, 0.0, false);
    setRow(1, tr("Mean |heading| (rad)"), 0.0, 0.0, false);
    setRow(2, tr("Min ST ref v (m/s)"), 0.0, 0.0, false, 2);
    setRow(3, tr("MPC solve rate"), 0.0, 0.0, false, 2);
}

void ExperimentComparePanel::setRow(int row,
                                    const QString& label,
                                    double current,
                                    double baseline,
                                    bool has_baseline,
                                    int precision)
{
    if (table_->item(row, 0) == nullptr) {
        table_->setItem(row, 0, new QTableWidgetItem(label));
        table_->setItem(row, 1, new QTableWidgetItem());
        table_->setItem(row, 2, new QTableWidgetItem());
    } else {
        table_->item(row, 0)->setText(label);
    }
    table_->item(row, 1)->setText(QString::number(current, 'f', precision));
    if (has_baseline) {
        table_->item(row, 2)->setText(QString::number(baseline, 'f', precision));
    } else {
        table_->item(row, 2)->setText(QStringLiteral("—"));
    }
}

void ExperimentComparePanel::updateCurrentRun(double mean_abs_cross_track,
                                              double mean_abs_heading_error,
                                              double min_st_ref_velocity,
                                              double mpc_solve_rate,
                                              quint64 sample_count)
{
    Q_UNUSED(sample_count);
    current_cross_track_ = mean_abs_cross_track;
    current_heading_ = mean_abs_heading_error;
    current_min_st_ = min_st_ref_velocity;
    current_mpc_rate_ = mpc_solve_rate;
    setRow(0, tr("Mean |cross-track| (m)"), current_cross_track_,
           baseline_cross_track_, has_baseline_);
    setRow(1, tr("Mean |heading| (rad)"), current_heading_,
           baseline_heading_, has_baseline_);
    setRow(2, tr("Min ST ref v (m/s)"), current_min_st_, baseline_min_st_, has_baseline_, 2);
    setRow(3, tr("MPC solve rate"), current_mpc_rate_, baseline_mpc_rate_, has_baseline_, 2);
}

void ExperimentComparePanel::updateBaseline(double mean_abs_cross_track,
                                            double mean_abs_heading_error,
                                            double min_st_ref_velocity,
                                            double mpc_solve_rate,
                                            std::size_t sample_count)
{
    has_baseline_ = true;
    baseline_cross_track_ = mean_abs_cross_track;
    baseline_heading_ = mean_abs_heading_error;
    baseline_min_st_ = min_st_ref_velocity;
    baseline_mpc_rate_ = mpc_solve_rate;
    status_label_->setText(tr("Baseline: %1 samples captured").arg(sample_count));
    setRow(0, tr("Mean |cross-track| (m)"), current_cross_track_,
           baseline_cross_track_, true);
    setRow(1, tr("Mean |heading| (rad)"), current_heading_,
           baseline_heading_, true);
    setRow(2, tr("Min ST ref v (m/s)"), current_min_st_, baseline_min_st_, true, 2);
    setRow(3, tr("MPC solve rate"), current_mpc_rate_, baseline_mpc_rate_, true, 2);
}

void ExperimentComparePanel::clearBaseline()
{
    has_baseline_ = false;
    status_label_->setText(tr("Baseline: not captured"));
    for (int row = 0; row < 4; ++row) {
        if (table_->item(row, 2) != nullptr) {
            table_->item(row, 2)->setText(QStringLiteral("—"));
        }
    }
}

}  // namespace fleetsim::ui
