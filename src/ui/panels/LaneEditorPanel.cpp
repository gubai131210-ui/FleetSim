#include "LaneEditorPanel.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

namespace fleetsim::ui {

LaneEditorPanel::LaneEditorPanel(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);

    auto* mode_box = new QGroupBox(tr("Lane Edit Mode"), this);
    auto* mode_layout = new QVBoxLayout(mode_box);
    edit_mode_ = new QCheckBox(tr("Enable lane editing"), mode_box);
    mode_layout->addWidget(edit_mode_);
    layout->addWidget(mode_box);

    auto* tool_box = new QGroupBox(tr("Tools"), this);
    auto* tool_layout = new QVBoxLayout(tool_box);

    auto* select_button = new QPushButton(tr("Select node"), tool_box);
    connect(select_button, &QPushButton::clicked, this, [this]() {
        current_tool_ = LaneEditorTool::Select;
        emit toolChanged(current_tool_);
    });
    tool_layout->addWidget(select_button);

    auto* add_node_button = new QPushButton(tr("Add node (click map)"), tool_box);
    connect(add_node_button, &QPushButton::clicked, this, [this]() {
        current_tool_ = LaneEditorTool::AddNode;
        emit toolChanged(current_tool_);
    });
    tool_layout->addWidget(add_node_button);
    layout->addWidget(tool_box);

    auto* node_box = new QGroupBox(tr("Nodes"), this);
    auto* node_layout = new QVBoxLayout(node_box);
    node_list_ = new QListWidget(node_box);
    connect(node_list_, &QListWidget::currentTextChanged, this, &LaneEditorPanel::nodeSelected);
    node_layout->addWidget(node_list_);

    auto* delete_node_button = new QPushButton(tr("Delete selected node"), node_box);
    connect(delete_node_button, &QPushButton::clicked, this, [this]() {
        const auto* item = node_list_->currentItem();
        if (item != nullptr) {
            emit deleteNodeRequested(item->text());
        }
    });
    node_layout->addWidget(delete_node_button);
    layout->addWidget(node_box);

    auto* edge_box = new QGroupBox(tr("Edges"), this);
    auto* edge_layout = new QVBoxLayout(edge_box);
    edge_list_ = new QListWidget(edge_box);
    edge_layout->addWidget(edge_list_);

    auto* connect_row = new QHBoxLayout();
    edge_from_combo_ = new QComboBox(edge_box);
    edge_to_combo_ = new QComboBox(edge_box);
    connect_row->addWidget(edge_from_combo_);
    connect_row->addWidget(new QLabel(tr("→"), edge_box));
    connect_row->addWidget(edge_to_combo_);
    edge_layout->addLayout(connect_row);

    bidirectional_check_ = new QCheckBox(tr("Bidirectional"), edge_box);
    edge_layout->addWidget(bidirectional_check_);

    auto* connect_button = new QPushButton(tr("Connect"), edge_box);
    connect(connect_button, &QPushButton::clicked, this, [this]() {
        if (edge_from_combo_->currentText().isEmpty() || edge_to_combo_->currentText().isEmpty()) {
            return;
        }
        emit connectEdgeRequested(
            edge_from_combo_->currentText(),
            edge_to_combo_->currentText(),
            bidirectional_check_->isChecked());
    });
    edge_layout->addWidget(connect_button);

    auto* delete_edge_button = new QPushButton(tr("Delete selected edge"), edge_box);
    connect(delete_edge_button, &QPushButton::clicked, this, [this]() {
        const int row = edge_list_->currentRow();
        if (row >= 0) {
            emit deleteEdgeRequested(row);
        }
    });
    edge_layout->addWidget(delete_edge_button);
    layout->addWidget(edge_box);

    layout->addStretch();

    connect(edit_mode_, &QCheckBox::toggled, this, &LaneEditorPanel::laneEditModeToggled);
}

void LaneEditorPanel::refreshFromLaneData(const domain::map::LaneMapData& lanes)
{
    rebuildNodeList(lanes);
    rebuildEdgeList(lanes);
    rebuildEdgeCombos(lanes);
}

void LaneEditorPanel::setSelectedNodeId(const QString& node_id)
{
    const auto items = node_list_->findItems(node_id, Qt::MatchExactly);
    if (!items.isEmpty()) {
        node_list_->setCurrentItem(items.front());
    }
}

void LaneEditorPanel::rebuildNodeList(const domain::map::LaneMapData& lanes)
{
    node_list_->clear();
    for (const domain::map::LaneNode& node : lanes.nodes) {
        node_list_->addItem(QString::fromStdString(node.id));
    }
}

void LaneEditorPanel::rebuildEdgeList(const domain::map::LaneMapData& lanes)
{
    edge_list_->clear();
    for (const domain::map::LaneEdge& edge : lanes.edges) {
        const QString label = QStringLiteral("%1 → %2%3")
                                  .arg(QString::fromStdString(edge.from),
                                       QString::fromStdString(edge.to),
                                       edge.bidirectional ? QStringLiteral(" (↔)") : QString());
        edge_list_->addItem(label);
    }
}

void LaneEditorPanel::rebuildEdgeCombos(const domain::map::LaneMapData& lanes)
{
    edge_from_combo_->clear();
    edge_to_combo_->clear();
    for (const domain::map::LaneNode& node : lanes.nodes) {
        const QString id = QString::fromStdString(node.id);
        edge_from_combo_->addItem(id);
        edge_to_combo_->addItem(id);
    }
}

}  // namespace fleetsim::ui
