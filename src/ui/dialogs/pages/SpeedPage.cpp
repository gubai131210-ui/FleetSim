#include "SpeedPage.h"

#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace fleetsim::ui {

SpeedPage::SpeedPage(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(
        tr("Longitudinal ST-Graph speed planning and peer trajectory prediction."), this));

    auto* form = new QFormLayout();
    speed_planner_combo_ = new QComboBox(this);
    speed_planner_combo_->addItem(tr("None"), QStringLiteral("none"));
    speed_planner_combo_->addItem(tr("ST-Graph"), QStringLiteral("st_graph"));
    form->addRow(tr("Speed planner"), speed_planner_combo_);

    prediction_combo_ = new QComboBox(this);
    prediction_combo_->addItem(tr("None (static peer path)"), QStringLiteral("none"));
    prediction_combo_->addItem(tr("Constant velocity"), QStringLiteral("constant_velocity"));
    form->addRow(tr("Prediction"), prediction_combo_);

    layout->addLayout(form);
    layout->addStretch();
}

QString SpeedPage::speedPlanner() const
{
    return speed_planner_combo_->currentData().toString();
}

void SpeedPage::setSpeedPlanner(const QString& speed_planner)
{
    const int index = speed_planner_combo_->findData(speed_planner);
    if (index >= 0) {
        speed_planner_combo_->setCurrentIndex(index);
    }
}

QString SpeedPage::prediction() const
{
    return prediction_combo_->currentData().toString();
}

void SpeedPage::setPrediction(const QString& prediction)
{
    const int index = prediction_combo_->findData(prediction);
    if (index >= 0) {
        prediction_combo_->setCurrentIndex(index);
    }
}

}  // namespace fleetsim::ui
