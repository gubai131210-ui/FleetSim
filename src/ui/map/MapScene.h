#pragma once

#include <QGraphicsScene>
#include <QGraphicsItemGroup>

namespace fleetsim::ui {

class ObstacleGraphicsItem;
class ObstacleOverlayItem;
class PathGraphicsItem;

class MapScene : public QGraphicsScene {
    Q_OBJECT

public:
    explicit MapScene(QObject* parent = nullptr);

    void setMapSizeM(double width_m, double height_m);
    double mapWidthM() const;
    double mapHeightM() const;

    PathGraphicsItem* pathItem();
    QGraphicsItemGroup* pathLayer();
    void clearPathLayer();

    ObstacleOverlayItem* obstacleOverlayItem();

    QGraphicsItemGroup* editorLayer();
    QGraphicsItemGroup* vehicleLayer();

    void clearEditorLayer();

private:
    void rebuildBackground();

    double map_width_m_{20.0};
    double map_height_m_{15.0};
    QGraphicsItemGroup* background_group_{nullptr};
    ObstacleOverlayItem* obstacle_overlay_{nullptr};
    PathGraphicsItem* path_item_{nullptr};
    QGraphicsItemGroup* path_layer_{nullptr};
    QGraphicsItemGroup* editor_layer_{nullptr};
    QGraphicsItemGroup* vehicle_layer_{nullptr};
};

}  // namespace fleetsim::ui
