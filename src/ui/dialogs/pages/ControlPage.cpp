#include "ControlPage.h"

#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace fleetsim::ui {

ControlPage::ControlPage(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(
        tr("Lateral path tracker (Pure Pursuit / Stanley / MPC)."), this));

    auto* form = new QFormLayout();
    tracker_combo_ = new QComboBox(this);
    tracker_combo_->addItem(tr("Auto (Pure Pursuit)"), QStringLiteral("auto"));
    tracker_combo_->addItem(tr("Pure Pursuit"), QStringLiteral("pure_pursuit"));
    tracker_combo_->addItem(tr("Stanley"), QStringLiteral("stanley"));
    tracker_combo_->addItem(tr("MPC (linear lateral)"), QStringLiteral("mpc"));
    form->addRow(tr("Tracker"), tracker_combo_);
    layout->addLayout(form);
    layout->addStretch();
}

QString ControlPage::tracker() const
{
    return tracker_combo_->currentData().toString();
}

void ControlPage::setTracker(const QString& tracker)
{
    const int index = tracker_combo_->findData(tracker);
    if (index >= 0) {
        tracker_combo_->setCurrentIndex(index);
    }
}

}  // namespace fleetsim::ui
