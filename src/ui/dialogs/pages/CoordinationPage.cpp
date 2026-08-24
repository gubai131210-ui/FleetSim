#include "CoordinationPage.h"

#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>

namespace fleetsim::ui {

CoordinationPage::CoordinationPage(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(
        tr("Multi-vehicle path coordination (Priority / CBS-lite / TimeWindow)."), this));

    auto* form = new QFormLayout();
    coordination_combo_ = new QComboBox(this);
    coordination_combo_->addItem(tr("Priority + TimeWindow"), QStringLiteral("priority"));
    coordination_combo_->addItem(tr("CBS-lite (bounded)"), QStringLiteral("cbs_lite"));
    coordination_combo_->addItem(tr("None (independent plans)"), QStringLiteral("none"));
    form->addRow(tr("Coordination"), coordination_combo_);

    cbs_fields_ = new QWidget(this);
    auto* cbs_form = new QFormLayout(cbs_fields_);
    cbs_depth_spin_ = new QSpinBox(cbs_fields_);
    cbs_depth_spin_->setRange(0, 100);
    cbs_depth_spin_->setValue(10);
    cbs_form->addRow(tr("CBS max depth"), cbs_depth_spin_);

    cbs_time_spin_ = new QSpinBox(cbs_fields_);
    cbs_time_spin_->setRange(1, 10000);
    cbs_time_spin_->setValue(100);
    cbs_time_spin_->setSuffix(tr(" ms"));
    cbs_form->addRow(tr("CBS time limit"), cbs_time_spin_);
    form->addRow(cbs_fields_);

    layout->addLayout(form);
    layout->addStretch();

    connect(coordination_combo_, &QComboBox::currentIndexChanged, this,
            [this]() { refreshCbsFieldsVisibility(); });
    refreshCbsFieldsVisibility();
}

QString CoordinationPage::coordination() const
{
    return coordination_combo_->currentData().toString();
}

int CoordinationPage::cbsMaxDepth() const
{
    return cbs_depth_spin_->value();
}

int CoordinationPage::cbsTimeLimitMs() const
{
    return cbs_time_spin_->value();
}

void CoordinationPage::setCoordination(const QString& coordination)
{
    const int index = coordination_combo_->findData(coordination);
    if (index >= 0) {
        coordination_combo_->setCurrentIndex(index);
    }
    refreshCbsFieldsVisibility();
}

void CoordinationPage::setCbsMaxDepth(int depth)
{
    cbs_depth_spin_->setValue(depth);
}

void CoordinationPage::setCbsTimeLimitMs(int milliseconds)
{
    cbs_time_spin_->setValue(milliseconds);
}

void CoordinationPage::refreshCbsFieldsVisibility()
{
    const bool cbs = coordination_combo_->currentData().toString() == QStringLiteral("cbs_lite");
    cbs_fields_->setVisible(cbs);
}

}  // namespace fleetsim::ui
