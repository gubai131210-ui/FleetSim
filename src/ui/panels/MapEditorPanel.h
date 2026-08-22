#pragma once

#include <QWidget>

namespace fleetsim::ui {

enum class EditorTool {
    Select,
    Rect,
    Polygon,
    StartPoint,
    EndPoint,
};

class MapEditorPanel : public QWidget {
    Q_OBJECT

public:
    explicit MapEditorPanel(QWidget* parent = nullptr);

    EditorTool currentTool() const { return current_tool_; }

signals:
    void toolChanged(fleetsim::ui::EditorTool tool);
    void undoRequested();
    void editModeToggled(bool enabled);

private:
    EditorTool current_tool_{EditorTool::Select};
};

}  // namespace fleetsim::ui
