#include "MapScene.h"

#include "graphics/ObstacleOverlayItem.h"
#include "graphics/PathGraphicsItem.h"

#include <QBrush>
#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QPen>

namespace fleetsim::ui {

MapScene::MapScene(QObject* parent)
    : QGraphicsScene(parent)
{
    background_group_ = new QGraphicsItemGroup();
    obstacle_overlay_ = new ObstacleOverlayItem();
    path_item_ = new PathGraphicsItem();
    vehicle_layer_ = new QGraphicsItemGroup();

    addItem(background_group_);
    addItem(obstacle_overlay_);
    addItem(path_item_);
    addItem(vehicle_layer_);

    rebuildBackground();
}

void MapScene::setMapSizeM(double width_m, double height_m)
{
    map_width_m_ = width_m;
    map_height_m_ = height_m;
    rebuildBackground();
}

double MapScene::mapWidthM() const
{
    return map_width_m_;
}

double MapScene::mapHeightM() const
{
    return map_height_m_;
}

PathGraphicsItem* MapScene::pathItem()
{
    return path_item_;
}

ObstacleOverlayItem* MapScene::obstacleOverlayItem()
{
    return obstacle_overlay_;
}

QGraphicsItemGroup* MapScene::vehicleLayer()
{
    return vehicle_layer_;
}

void MapScene::rebuildBackground()
{
    setSceneRect(0.0, 0.0, map_width_m_, map_height_m_);

    for (QGraphicsItem* child : background_group_->childItems()) {
        background_group_->removeFromGroup(child);
        delete child;
    }

    QPen border_pen(Qt::darkGray);
    border_pen.setWidthF(0.05);

    auto* border = new QGraphicsRectItem(0.0, 0.0, map_width_m_, map_height_m_);
    border->setPen(border_pen);
    border->setBrush(QBrush(Qt::white));
    background_group_->addToGroup(border);

    QPen grid_pen(QColor(220, 220, 220));
    grid_pen.setWidthF(0.01);
    for (double x = 0.0; x <= map_width_m_; x += 1.0) {
        auto* line = new QGraphicsLineItem(x, 0.0, x, map_height_m_);
        line->setPen(grid_pen);
        background_group_->addToGroup(line);
    }
    for (double y = 0.0; y <= map_height_m_; y += 1.0) {
        auto* line = new QGraphicsLineItem(0.0, y, map_width_m_, y);
        line->setPen(grid_pen);
        background_group_->addToGroup(line);
    }
}

}  // namespace fleetsim::ui
