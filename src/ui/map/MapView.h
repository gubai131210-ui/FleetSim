#pragma once

#include "panels/MapEditorPanel.h"

#include <QGraphicsView>
#include <QPointF>
#include <QVector>

namespace fleetsim::ui {

class MapScene;

class MapView : public QGraphicsView {
    Q_OBJECT

public:
    explicit MapView(QWidget* parent = nullptr);

    MapScene* mapScene();

    void setMapSizeM(double width_m, double height_m);
    void setEditModeEnabled(bool enabled);
    void setEditorTool(EditorTool tool);
    void setLaneEditModeEnabled(bool enabled);
    void setPlacingLaneNode(bool enabled);

signals:
    void goalRequested(double x_m, double y_m);
    void rectObstacleCreated(double x, double y, double width, double height);
    void polygonObstacleCreated(const QVector<QPointF>& vertices);
    void startPointRequested(double x_m, double y_m);
    void endPointRequested(double x_m, double y_m);
    void laneNodePlaceRequested(double x_m, double y_m);

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    MapScene* scene_{nullptr};
    bool edit_mode_enabled_{false};
    bool lane_edit_mode_enabled_{false};
    bool placing_lane_node_{false};
    EditorTool editor_tool_{EditorTool::Select};
    bool drawing_rect_{false};
    QPointF rect_start_;
    QGraphicsRectItem* preview_rect_{nullptr};
    QVector<QPointF> polygon_points_;
    QGraphicsPathItem* preview_polygon_{nullptr};
};

}  // namespace fleetsim::ui
