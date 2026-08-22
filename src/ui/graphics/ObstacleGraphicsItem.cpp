#include "ObstacleGraphicsItem.h"

#include <QPainter>

#include <algorithm>

namespace fleetsim::ui {

ObstacleGraphicsItem::ObstacleGraphicsItem(const domain::map::Obstacle& obstacle, QGraphicsItem* parent)
    : QGraphicsItem(parent)
    , obstacle_(obstacle)
{
    setZValue(5.0);
}

void ObstacleGraphicsItem::setObstacle(const domain::map::Obstacle& obstacle)
{
    prepareGeometryChange();
    obstacle_ = obstacle;
    update();
}

QRectF ObstacleGraphicsItem::boundingRect() const
{
    if (obstacle_.type == domain::map::ObstacleType::Rect) {
        return QRectF(obstacle_.rect.x,
                      obstacle_.rect.y,
                      obstacle_.rect.width,
                      obstacle_.rect.height);
    }

    if (obstacle_.polygon.vertices.empty()) {
        return {};
    }

    double min_x = obstacle_.polygon.vertices.front().first;
    double max_x = min_x;
    double min_y = obstacle_.polygon.vertices.front().second;
    double max_y = min_y;
    for (const auto& vertex : obstacle_.polygon.vertices) {
        min_x = std::min(min_x, vertex.first);
        max_x = std::max(max_x, vertex.first);
        min_y = std::min(min_y, vertex.second);
        max_y = std::max(max_y, vertex.second);
    }
    return QRectF(min_x, min_y, max_x - min_x, max_y - min_y);
}

void ObstacleGraphicsItem::paint(QPainter* painter,
                                 const QStyleOptionGraphicsItem* /*option*/,
                                 QWidget* /*widget*/)
{
    QPen pen(QColor(180, 60, 60), 0.05);
    pen.setCosmetic(true);
    painter->setPen(pen);
    painter->setBrush(QColor(200, 80, 80, 120));

    if (obstacle_.type == domain::map::ObstacleType::Rect) {
        painter->drawRect(QRectF(obstacle_.rect.x,
                                 obstacle_.rect.y,
                                 obstacle_.rect.width,
                                 obstacle_.rect.height));
        return;
    }

    QPolygonF polygon;
    for (const auto& vertex : obstacle_.polygon.vertices) {
        polygon << QPointF(vertex.first, vertex.second);
    }
    if (polygon.size() >= 3) {
        painter->drawPolygon(polygon);
    }
}

}  // namespace fleetsim::ui
