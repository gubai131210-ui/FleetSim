#pragma once

#include "domain/map/MapData.h"

#include <QGraphicsItem>

namespace fleetsim::ui {

class ObstacleGraphicsItem : public QGraphicsItem {
public:
    explicit ObstacleGraphicsItem(const domain::map::Obstacle& obstacle, QGraphicsItem* parent = nullptr);

    void setObstacle(const domain::map::Obstacle& obstacle);
    const domain::map::Obstacle& obstacle() const { return obstacle_; }

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    domain::map::Obstacle obstacle_;
};

}  // namespace fleetsim::ui
