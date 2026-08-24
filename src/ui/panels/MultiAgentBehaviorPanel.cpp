#include "MultiAgentBehaviorPanel.h"

#include <QHeaderView>
#include <QTableWidget>
#include <QVBoxLayout>

namespace fleetsim::ui {

MultiAgentBehaviorPanel::MultiAgentBehaviorPanel(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    table_ = new QTableWidget(0, 5, this);
    table_->setHorizontalHeaderLabels(
        {tr("Agent"), tr("Tree"), tr("Active node"), tr("Status"), tr("path_valid")});
    table_->horizontalHeader()->setStretchLastSection(true);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    layout->addWidget(table_);
}

void MultiAgentBehaviorPanel::updateAgents(const QVector<MultiAgentBehaviorRow>& rows)
{
    table_->setRowCount(rows.size());
    for (int row = 0; row < rows.size(); ++row) {
        const MultiAgentBehaviorRow& entry = rows[row];
        table_->setItem(row, 0, new QTableWidgetItem(entry.agent_id));
        table_->setItem(row, 1, new QTableWidgetItem(entry.tree_name));
        table_->setItem(row, 2, new QTableWidgetItem(entry.active_node));
        table_->setItem(row, 3, new QTableWidgetItem(entry.node_status));
        table_->setItem(row, 4,
                        new QTableWidgetItem(entry.path_valid ? tr("true") : tr("false")));
    }
}

}  // namespace fleetsim::ui
