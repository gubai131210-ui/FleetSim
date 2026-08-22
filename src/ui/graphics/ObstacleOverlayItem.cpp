#include "ObstacleOverlayItem.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace fleetsim::ui {

ObstacleOverlayItem::ObstacleOverlayItem(QGraphicsItem* parent)
    : QGraphicsItem(parent)
{
    setZValue(5.0);
}

void ObstacleOverlayItem::setOccupiedCells(const QVector<QPointF>& cell_centers,
                                           double cell_size_m)
{
    prepareGeometryChange();
    cell_centers_ = cell_centers;
    cell_size_m_ = cell_size_m;
    update();
}

QRectF ObstacleOverlayItem::boundingRect() const
{
    if (cell_centers_.isEmpty()) {
        return {};
    }

    const double half = cell_size_m_ * 0.5;
    QRectF bounds;
    for (const QPointF& center : cell_centers_) {
        bounds |= QRectF(center.x() - half, center.y() - half, cell_size_m_, cell_size_m_);
    }
    return bounds;
}

void ObstacleOverlayItem::paint(QPainter* painter,
                                const QStyleOptionGraphicsItem* option,
                                QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(120, 120, 120, 120));

    const double half = cell_size_m_ * 0.5;
    for (const QPointF& center : cell_centers_) {
        painter->drawRect(QRectF(center.x() - half, center.y() - half, cell_size_m_, cell_size_m_));
    }
}

}  // namespace fleetsim::ui
