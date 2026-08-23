#include "PlannerTrackerDialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace fleetsim::ui {

PlannerTrackerDialog::PlannerTrackerDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Planner / Tracker"));
    resize(400, 240);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(
        tr("Path planner, path tracker, and multi-vehicle coordination."), this));

    auto* form = new QFormLayout();

    planner_combo_ = new QComboBox(this);
    planner_combo_->addItem(tr("Auto (bicycle→Hybrid A*)"), QStringLiteral("auto"));
    planner_combo_->addItem(tr("A* (grid)"), QStringLiteral("astar"));
    planner_combo_->addItem(tr("Hybrid A*"), QStringLiteral("hybrid_astar"));
    form->addRow(tr("Planner"), planner_combo_);

    tracker_combo_ = new QComboBox(this);
    tracker_combo_->addItem(tr("Auto (Pure Pursuit)"), QStringLiteral("auto"));
    tracker_combo_->addItem(tr("Pure Pursuit"), QStringLiteral("pure_pursuit"));
    tracker_combo_->addItem(tr("Stanley"), QStringLiteral("stanley"));
    tracker_combo_->addItem(tr("MPC (linear lateral)"), QStringLiteral("mpc"));
    form->addRow(tr("Tracker"), tracker_combo_);

    coordination_combo_ = new QComboBox(this);
    coordination_combo_->addItem(tr("Priority + TimeWindow"), QStringLiteral("priority"));
    coordination_combo_->addItem(tr("None (independent plans)"), QStringLiteral("none"));
    form->addRow(tr("Coordination"), coordination_combo_);

    layout->addLayout(form);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

PlannerTrackerSettings PlannerTrackerDialog::settings() const
{
    PlannerTrackerSettings out;
    out.planner = planner_combo_->currentData().toString();
    out.tracker = tracker_combo_->currentData().toString();
    out.coordination = coordination_combo_->currentData().toString();
    return out;
}

void PlannerTrackerDialog::setSettings(const PlannerTrackerSettings& settings)
{
    const int planner_index = planner_combo_->findData(settings.planner);
    if (planner_index >= 0) {
        planner_combo_->setCurrentIndex(planner_index);
    }
    const int tracker_index = tracker_combo_->findData(settings.tracker);
    if (tracker_index >= 0) {
        tracker_combo_->setCurrentIndex(tracker_index);
    }
    const int coordination_index = coordination_combo_->findData(settings.coordination);
    if (coordination_index >= 0) {
        coordination_combo_->setCurrentIndex(coordination_index);
    }
}

}  // namespace fleetsim::ui
