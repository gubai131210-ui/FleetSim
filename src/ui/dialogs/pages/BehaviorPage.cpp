#include "BehaviorPage.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>

namespace fleetsim::ui {

BehaviorPage::BehaviorPage(QWidget* parent)
    : QWidget(parent)
{
    auto* form = new QFormLayout(this);

    behavior_mode_combo_ = new QComboBox(this);
    behavior_mode_combo_->addItem(tr("Legacy (AgentPhase)"), QStringLiteral("legacy"));
    behavior_mode_combo_->addItem(tr("Behavior Tree"), QStringLiteral("bt"));
    form->addRow(tr("Behavior mode"), behavior_mode_combo_);

    auto* path_row = new QWidget(this);
    auto* path_layout = new QHBoxLayout(path_row);
    path_layout->setContentsMargins(0, 0, 0, 0);
    tree_path_edit_ = new QLineEdit(path_row);
    tree_path_edit_->setPlaceholderText(tr("navigate_replan_recovery.json"));
    browse_button_ = new QPushButton(tr("Browse…"), path_row);
    path_layout->addWidget(tree_path_edit_, 1);
    path_layout->addWidget(browse_button_);
    form->addRow(tr("Behavior tree"), path_row);

    replan_hz_spin_ = new QDoubleSpinBox(this);
    replan_hz_spin_->setRange(0.1, 20.0);
    replan_hz_spin_->setSingleStep(0.1);
    replan_hz_spin_->setDecimals(2);
    replan_hz_spin_->setValue(1.0);
    form->addRow(tr("Replan rate (Hz)"), replan_hz_spin_);

    recovery_enabled_ = new QCheckBox(tr("Enable recovery wait"), this);
    recovery_enabled_->setChecked(true);
    form->addRow(QString(), recovery_enabled_);

    recovery_ticks_spin_ = new QSpinBox(this);
    recovery_ticks_spin_->setRange(0, 10000);
    recovery_ticks_spin_->setValue(20);
    form->addRow(tr("Recovery wait ticks"), recovery_ticks_spin_);

    connect(browse_button_, &QPushButton::clicked, this, &BehaviorPage::onBrowseClicked);
    connect(recovery_enabled_, &QCheckBox::toggled, recovery_ticks_spin_, &QWidget::setEnabled);
}

QString BehaviorPage::behaviorMode() const
{
    return behavior_mode_combo_->currentData().toString();
}

QString BehaviorPage::behaviorTreePath() const
{
    return tree_path_edit_->text().trimmed();
}

double BehaviorPage::replanHz() const
{
    return replan_hz_spin_->value();
}

int BehaviorPage::recoveryWaitTicks() const
{
    if (!recovery_enabled_->isChecked()) {
        return 0;
    }
    return recovery_ticks_spin_->value();
}

bool BehaviorPage::recoveryEnabled() const
{
    return recovery_enabled_->isChecked();
}

void BehaviorPage::setBehaviorMode(const QString& mode)
{
    const int index = behavior_mode_combo_->findData(mode);
    if (index >= 0) {
        behavior_mode_combo_->setCurrentIndex(index);
    }
}

void BehaviorPage::setBehaviorTreePath(const QString& path)
{
    tree_path_edit_->setText(path);
}

void BehaviorPage::setReplanHz(double hz)
{
    replan_hz_spin_->setValue(hz);
}

void BehaviorPage::setRecoveryWaitTicks(int ticks)
{
    recovery_enabled_->setChecked(ticks > 0);
    recovery_ticks_spin_->setValue(ticks > 0 ? ticks : 20);
}

void BehaviorPage::setRecoveryEnabled(bool enabled)
{
    recovery_enabled_->setChecked(enabled);
}

void BehaviorPage::onBrowseClicked()
{
    const QString path = QFileDialog::getOpenFileName(
        this,
        tr("Select behavior tree JSON"),
        QString(),
        tr("Behavior trees (*.json);;All files (*.*)"));
    if (!path.isEmpty()) {
        tree_path_edit_->setText(path);
    }
    emit browseTreeFileRequested();
}

}  // namespace fleetsim::ui
