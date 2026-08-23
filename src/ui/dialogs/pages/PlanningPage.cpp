#include "PlanningPage.h"

#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace fleetsim::ui {

PlanningPage::PlanningPage(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(
        tr("Spatial path planner selection (A* / Hybrid A*)."), this));

    auto* form = new QFormLayout();
    planner_combo_ = new QComboBox(this);
    planner_combo_->addItem(tr("Auto (bicycle→Hybrid A*)"), QStringLiteral("auto"));
    planner_combo_->addItem(tr("A* (grid)"), QStringLiteral("astar"));
    planner_combo_->addItem(tr("Hybrid A*"), QStringLiteral("hybrid_astar"));
    form->addRow(tr("Planner"), planner_combo_);
    layout->addLayout(form);
    layout->addStretch();
}

QString PlanningPage::planner() const
{
    return planner_combo_->currentData().toString();
}

void PlanningPage::setPlanner(const QString& planner)
{
    const int index = planner_combo_->findData(planner);
    if (index >= 0) {
        planner_combo_->setCurrentIndex(index);
    }
}

}  // namespace fleetsim::ui
