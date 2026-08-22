#include "PathGraphicsItem.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace fleetsim::ui {

PathGraphicsItem::PathGraphicsItem(QGraphicsItem* parent)
    : QGraphicsItem(parent)
{
    setZValue(20.0);
}

void PathGraphicsItem::setPathPoints(const QVector<QPointF>& points)
{
    prepareGeometryChange();
    points_ = points;
    update();
}

void PathGraphicsItem::setPathColor(const QColor& color)
{
    path_color_ = color;
    update();
}

QRectF PathGraphicsItem::boundingRect() const
{
    if (points_.isEmpty()) {
        return {};
    }

    QRectF bounds(points_.first(), QSizeF(0.0, 0.0));
    for (const QPointF& point : points_) {
        bounds |= QRectF(point, QSizeF(0.0, 0.0));
    }
    return bounds.adjusted(-0.2, -0.2, 0.2, 0.2);
}

void PathGraphicsItem::paint(QPainter* painter,
                             const QStyleOptionGraphicsItem* option,
                             QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    if (points_.size() < 2) {
        return;
    }

    painter->setRenderHint(QPainter::Antialiasing, true);
    QPen pen(path_color_, 0.08);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter->setPen(pen);

    for (int i = 1; i < points_.size(); ++i) {
        painter->drawLine(points_[i - 1], points_[i]);
    }
}

}  // namespace fleetsim::ui
