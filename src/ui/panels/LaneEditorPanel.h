#pragma once

#include "domain/map/LaneTypes.h"

#include <QWidget>

namespace fleetsim::ui {

enum class LaneEditorTool {
    Select,
    AddNode,
};

class LaneEditorPanel : public QWidget {
    Q_OBJECT

public:
    explicit LaneEditorPanel(QWidget* parent = nullptr);

    LaneEditorTool currentTool() const { return current_tool_; }

    void refreshFromLaneData(const domain::map::LaneMapData& lanes);
    void setSelectedNodeId(const QString& node_id);

signals:
    void laneEditModeToggled(bool enabled);
    void toolChanged(fleetsim::ui::LaneEditorTool tool);
    void nodeSelected(const QString& node_id);
    void connectEdgeRequested(const QString& from_id,
                              const QString& to_id,
                              bool bidirectional);
    void deleteNodeRequested(const QString& node_id);
    void deleteEdgeRequested(int edge_index);

private:
    void rebuildNodeList(const domain::map::LaneMapData& lanes);
    void rebuildEdgeList(const domain::map::LaneMapData& lanes);
    void rebuildEdgeCombos(const domain::map::LaneMapData& lanes);

    LaneEditorTool current_tool_{LaneEditorTool::Select};
    class QCheckBox* edit_mode_{nullptr};
    class QListWidget* node_list_{nullptr};
    class QListWidget* edge_list_{nullptr};
    class QComboBox* edge_from_combo_{nullptr};
    class QComboBox* edge_to_combo_{nullptr};
    class QCheckBox* bidirectional_check_{nullptr};
};

}  // namespace fleetsim::ui
