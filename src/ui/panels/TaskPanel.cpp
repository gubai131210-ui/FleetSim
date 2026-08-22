#include "TaskPanel.h"

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

namespace fleetsim::ui {

TaskPanel::TaskPanel(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);

    task_table_ = new QTableWidget(0, 3, this);
    task_table_->setHorizontalHeaderLabels({tr("ID"), tr("Pickup"), tr("Status")});
    task_table_->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(task_table_);

    auto* add_box = new QGroupBox(tr("Add Task"), this);
    auto* form = new QFormLayout(add_box);
    auto* pickup_x = new QDoubleSpinBox(add_box);
    auto* pickup_y = new QDoubleSpinBox(add_box);
    auto* dropoff_x = new QDoubleSpinBox(add_box);
    auto* dropoff_y = new QDoubleSpinBox(add_box);
    for (QDoubleSpinBox* spin : {pickup_x, pickup_y, dropoff_x, dropoff_y}) {
        spin->setRange(0.0, 100.0);
        spin->setDecimals(2);
    }
    pickup_x->setValue(3.0);
    pickup_y->setValue(2.0);
    dropoff_x->setValue(15.0);
    dropoff_y->setValue(10.0);
    form->addRow(tr("Pickup X"), pickup_x);
    form->addRow(tr("Pickup Y"), pickup_y);
    form->addRow(tr("Dropoff X"), dropoff_x);
    form->addRow(tr("Dropoff Y"), dropoff_y);

    auto* add_button = new QPushButton(tr("Add Task"), add_box);
    form->addRow(add_button);
    layout->addWidget(add_box);

    connect(add_button, &QPushButton::clicked, this, [this, pickup_x, pickup_y, dropoff_x, dropoff_y]() {
        emit addTaskRequested(
            pickup_x->value(), pickup_y->value(), dropoff_x->value(), dropoff_y->value());
    });
}

void TaskPanel::refreshTasks(const QStringList& lines)
{
    task_table_->setRowCount(lines.size());
    for (int row = 0; row < lines.size(); ++row) {
        const QStringList parts = lines.at(row).split('|');
        for (int col = 0; col < qMin(3, parts.size()); ++col) {
            task_table_->setItem(row, col, new QTableWidgetItem(parts.at(col).trimmed()));
        }
    }
}

}  // namespace fleetsim::ui
