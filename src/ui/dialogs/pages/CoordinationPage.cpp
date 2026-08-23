#include "CoordinationPage.h"

#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace fleetsim::ui {

CoordinationPage::CoordinationPage(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(
        tr("Multi-vehicle path coordination (Priority / TimeWindow)."), this));

    auto* form = new QFormLayout();
    coordination_combo_ = new QComboBox(this);
    coordination_combo_->addItem(tr("Priority + TimeWindow"), QStringLiteral("priority"));
    coordination_combo_->addItem(tr("None (independent plans)"), QStringLiteral("none"));
    form->addRow(tr("Coordination"), coordination_combo_);
    layout->addLayout(form);
    layout->addStretch();
}

QString CoordinationPage::coordination() const
{
    return coordination_combo_->currentData().toString();
}

void CoordinationPage::setCoordination(const QString& coordination)
{
    const int index = coordination_combo_->findData(coordination);
    if (index >= 0) {
        coordination_combo_->setCurrentIndex(index);
    }
}

}  // namespace fleetsim::ui
