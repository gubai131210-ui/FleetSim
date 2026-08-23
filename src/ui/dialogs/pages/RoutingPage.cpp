#include "RoutingPage.h"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace fleetsim::ui {

RoutingPage::RoutingPage(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(
        tr("Lane-level routing strategy (graph / hybrid first-last mile)."), this));

    auto* form = new QFormLayout();
    routing_mode_combo_ = new QComboBox(this);
    routing_mode_combo_->addItem(tr("Freespace (grid A*)"), QStringLiteral("freespace"));
    routing_mode_combo_->addItem(tr("Lane graph only"), QStringLiteral("lane_graph"));
    routing_mode_combo_->addItem(tr("Hybrid (first / lane / last)"), QStringLiteral("hybrid"));
    form->addRow(tr("Routing mode"), routing_mode_combo_);

    snap_radius_spin_ = new QDoubleSpinBox(this);
    snap_radius_spin_->setRange(0.1, 10.0);
    snap_radius_spin_->setSingleStep(0.1);
    snap_radius_spin_->setDecimals(2);
    snap_radius_spin_->setValue(1.0);
    snap_radius_spin_->setSuffix(tr(" m"));
    form->addRow(tr("Lane snap radius"), snap_radius_spin_);

    first_last_planner_combo_ = new QComboBox(this);
    first_last_planner_combo_->addItem(tr("Same as Planning page"), QString());
    first_last_planner_combo_->addItem(tr("A* (grid)"), QStringLiteral("astar"));
    first_last_planner_combo_->addItem(tr("Hybrid A*"), QStringLiteral("hybrid_astar"));
    form->addRow(tr("First/Last mile planner"), first_last_planner_combo_);

    layout->addLayout(form);
    layout->addStretch();
}

QString RoutingPage::routingMode() const
{
    return routing_mode_combo_->currentData().toString();
}

double RoutingPage::laneSnapRadiusM() const
{
    return snap_radius_spin_->value();
}

QString RoutingPage::firstLastPlanner() const
{
    return first_last_planner_combo_->currentData().toString();
}

void RoutingPage::setRoutingMode(const QString& mode)
{
    const int index = routing_mode_combo_->findData(mode);
    if (index >= 0) {
        routing_mode_combo_->setCurrentIndex(index);
    }
}

void RoutingPage::setLaneSnapRadiusM(double radius_m)
{
    snap_radius_spin_->setValue(radius_m);
}

void RoutingPage::setFirstLastPlanner(const QString& planner)
{
    const int index = first_last_planner_combo_->findData(planner);
    if (index >= 0) {
        first_last_planner_combo_->setCurrentIndex(index);
    }
}

}  // namespace fleetsim::ui
