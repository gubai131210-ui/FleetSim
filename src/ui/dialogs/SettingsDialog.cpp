#include "SettingsDialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace fleetsim::ui {

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Settings"));
    resize(360, 220);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(tr("Vehicle model & scheduling"), this));

    auto* form = new QFormLayout();
    model_combo_ = new QComboBox(this);
    model_combo_->addItem(tr("Diff Drive"), QStringLiteral("diff_drive"));
    model_combo_->addItem(tr("Bicycle"), QStringLiteral("bicycle"));
    form->addRow(tr("Vehicle model"), model_combo_);

    wheelbase_spin_ = new QDoubleSpinBox(this);
    wheelbase_spin_->setRange(0.1, 5.0);
    wheelbase_spin_->setSingleStep(0.1);
    wheelbase_spin_->setDecimals(2);
    wheelbase_spin_->setValue(0.9);
    form->addRow(tr("Wheelbase (m)"), wheelbase_spin_);

    max_steering_spin_ = new QDoubleSpinBox(this);
    max_steering_spin_->setRange(0.05, 1.2);
    max_steering_spin_->setSingleStep(0.05);
    max_steering_spin_->setDecimals(2);
    max_steering_spin_->setValue(0.6);
    form->addRow(tr("Max steering (rad)"), max_steering_spin_);

    assigner_combo_ = new QComboBox(this);
    assigner_combo_->addItem(tr("Greedy"), QStringLiteral("greedy"));
    assigner_combo_->addItem(tr("Hungarian"), QStringLiteral("hungarian"));
    form->addRow(tr("Task assigner"), assigner_combo_);

    layout->addLayout(form);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);

    connect(model_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int) { rebuildModelDependentEnabled(); });
    rebuildModelDependentEnabled();
}

void SettingsDialog::rebuildModelDependentEnabled()
{
    const bool bicycle = model_combo_->currentData().toString() == QStringLiteral("bicycle");
    wheelbase_spin_->setEnabled(bicycle);
    max_steering_spin_->setEnabled(bicycle);
}

SimulationSettings SettingsDialog::settings() const
{
    SimulationSettings out;
    out.vehicle_model = model_combo_->currentData().toString();
    out.wheelbase_m = wheelbase_spin_->value();
    out.max_steering_rad = max_steering_spin_->value();
    out.assigner = assigner_combo_->currentData().toString();
    return out;
}

void SettingsDialog::setSettings(const SimulationSettings& settings)
{
    const int model_index = model_combo_->findData(settings.vehicle_model);
    if (model_index >= 0) {
        model_combo_->setCurrentIndex(model_index);
    }
    wheelbase_spin_->setValue(settings.wheelbase_m);
    max_steering_spin_->setValue(settings.max_steering_rad);
    const int assigner_index = assigner_combo_->findData(settings.assigner);
    if (assigner_index >= 0) {
        assigner_combo_->setCurrentIndex(assigner_index);
    }
    rebuildModelDependentEnabled();
}

}  // namespace fleetsim::ui
