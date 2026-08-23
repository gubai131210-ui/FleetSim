#include "AlgorithmWorkbenchDialog.h"

#include "ControlPage.h"
#include "CoordinationPage.h"
#include "PlanningPage.h"
#include "SpeedPage.h"

#include <QDialogButtonBox>
#include <QTabWidget>
#include <QVBoxLayout>

namespace fleetsim::ui {

AlgorithmWorkbenchDialog::AlgorithmWorkbenchDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Algorithm Workbench"));
    resize(460, 320);

    auto* layout = new QVBoxLayout(this);
    tabs_ = new QTabWidget(this);
    planning_page_ = new PlanningPage(tabs_);
    control_page_ = new ControlPage(tabs_);
    speed_page_ = new SpeedPage(tabs_);
    coordination_page_ = new CoordinationPage(tabs_);

    tabs_->addTab(planning_page_, tr("Planning"));
    tabs_->addTab(control_page_, tr("Control"));
    tabs_->addTab(speed_page_, tr("Speed"));
    tabs_->addTab(coordination_page_, tr("Coordination"));
    layout->addWidget(tabs_);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

AlgorithmWorkbenchSettings AlgorithmWorkbenchDialog::settings() const
{
    AlgorithmWorkbenchSettings out;
    out.planner = planning_page_->planner();
    out.tracker = control_page_->tracker();
    out.speed_planner = speed_page_->speedPlanner();
    out.prediction = speed_page_->prediction();
    out.coordination = coordination_page_->coordination();
    return out;
}

void AlgorithmWorkbenchDialog::setSettings(const AlgorithmWorkbenchSettings& settings)
{
    planning_page_->setPlanner(settings.planner);
    control_page_->setTracker(settings.tracker);
    speed_page_->setSpeedPlanner(settings.speed_planner);
    speed_page_->setPrediction(settings.prediction);
    coordination_page_->setCoordination(settings.coordination);
}

}  // namespace fleetsim::ui
