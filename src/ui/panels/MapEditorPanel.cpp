#include "MapEditorPanel.h"

#include <QCheckBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

namespace fleetsim::ui {

MapEditorPanel::MapEditorPanel(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);

    auto* mode_box = new QGroupBox(tr("Edit Mode"), this);
    auto* mode_layout = new QVBoxLayout(mode_box);
    auto* edit_mode = new QCheckBox(tr("Enable map editing"), mode_box);
    mode_layout->addWidget(edit_mode);
    layout->addWidget(mode_box);

    auto* tool_box = new QGroupBox(tr("Tools"), this);
    auto* tool_layout = new QVBoxLayout(tool_box);

    auto add_tool_button = [this, tool_layout](const QString& label, EditorTool tool) {
        auto* button = new QPushButton(label, tool_box);
        connect(button, &QPushButton::clicked, this, [this, tool]() {
            current_tool_ = tool;
            emit toolChanged(tool);
        });
        tool_layout->addWidget(button);
    };

    add_tool_button(tr("Select"), EditorTool::Select);
    add_tool_button(tr("Rectangle obstacle"), EditorTool::Rect);
    add_tool_button(tr("Polygon obstacle"), EditorTool::Polygon);
    add_tool_button(tr("Set start pose"), EditorTool::StartPoint);
    add_tool_button(tr("Set goal"), EditorTool::EndPoint);

    auto* undo_button = new QPushButton(tr("Undo (Ctrl+Z)"), tool_box);
    connect(undo_button, &QPushButton::clicked, this, &MapEditorPanel::undoRequested);
    tool_layout->addWidget(undo_button);

    layout->addWidget(tool_box);
    layout->addStretch();

    connect(edit_mode, &QCheckBox::toggled, this, &MapEditorPanel::editModeToggled);
}

}  // namespace fleetsim::ui
