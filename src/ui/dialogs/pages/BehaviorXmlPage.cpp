#include "BehaviorXmlPage.h"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace fleetsim::ui {

BehaviorXmlPage::BehaviorXmlPage(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(
        tr("Nav2-style BT XML and motion recovery defaults (Spin / BackUp / RoundRobin)."),
        this));

    auto* form = new QFormLayout();
    bt_format_combo_ = new QComboBox(this);
    bt_format_combo_->addItem(tr("JSON (Phase 9)"), QStringLiteral("json"));
    bt_format_combo_->addItem(tr("XML (Nav2 subset)"), QStringLiteral("xml"));
    form->addRow(tr("BT format"), bt_format_combo_);

    auto* path_row = new QWidget(this);
    auto* path_layout = new QHBoxLayout(path_row);
    path_layout->setContentsMargins(0, 0, 0, 0);
    tree_path_edit_ = new QLineEdit(path_row);
    tree_path_edit_->setPlaceholderText(tr("navigate_spin_backup_recovery.xml"));
    browse_button_ = new QPushButton(tr("Browse…"), path_row);
    path_layout->addWidget(tree_path_edit_, 1);
    path_layout->addWidget(browse_button_);
    form->addRow(tr("Tree path"), path_row);

    spin_rad_spin_ = new QDoubleSpinBox(this);
    spin_rad_spin_->setRange(0.1, 6.28);
    spin_rad_spin_->setDecimals(3);
    spin_rad_spin_->setSingleStep(0.1);
    spin_rad_spin_->setValue(1.5708);
    form->addRow(tr("Spin (rad)"), spin_rad_spin_);

    backup_dist_spin_ = new QDoubleSpinBox(this);
    backup_dist_spin_->setRange(0.05, 5.0);
    backup_dist_spin_->setDecimals(2);
    backup_dist_spin_->setSingleStep(0.05);
    backup_dist_spin_->setValue(0.3);
    form->addRow(tr("BackUp distance (m)"), backup_dist_spin_);

    backup_speed_spin_ = new QDoubleSpinBox(this);
    backup_speed_spin_->setRange(0.01, 2.0);
    backup_speed_spin_->setDecimals(2);
    backup_speed_spin_->setSingleStep(0.05);
    backup_speed_spin_->setValue(0.1);
    form->addRow(tr("BackUp speed (m/s)"), backup_speed_spin_);

    layout->addLayout(form);
    layout->addStretch();

    connect(browse_button_, &QPushButton::clicked, this, &BehaviorXmlPage::onBrowseTree);
}

QString BehaviorXmlPage::btFormat() const
{
    return bt_format_combo_->currentData().toString();
}

QString BehaviorXmlPage::behaviorTreePath() const
{
    return tree_path_edit_->text().trimmed();
}

double BehaviorXmlPage::spinRad() const
{
    return spin_rad_spin_->value();
}

double BehaviorXmlPage::backupDistM() const
{
    return backup_dist_spin_->value();
}

double BehaviorXmlPage::backupSpeedMps() const
{
    return backup_speed_spin_->value();
}

void BehaviorXmlPage::setBtFormat(const QString& format)
{
    const int index = bt_format_combo_->findData(format);
    if (index >= 0) {
        bt_format_combo_->setCurrentIndex(index);
    }
}

void BehaviorXmlPage::setBehaviorTreePath(const QString& path)
{
    tree_path_edit_->setText(path);
}

void BehaviorXmlPage::setSpinRad(double rad)
{
    spin_rad_spin_->setValue(rad);
}

void BehaviorXmlPage::setBackupDistM(double meters)
{
    backup_dist_spin_->setValue(meters);
}

void BehaviorXmlPage::setBackupSpeedMps(double meters_per_second)
{
    backup_speed_spin_->setValue(meters_per_second);
}

void BehaviorXmlPage::onBrowseTree()
{
    const QString path = QFileDialog::getOpenFileName(
        this,
        tr("Select behavior tree"),
        QString(),
        tr("Behavior trees (*.xml *.json);;All files (*.*)"));
    if (!path.isEmpty()) {
        tree_path_edit_->setText(path);
    }
}

}  // namespace fleetsim::ui
