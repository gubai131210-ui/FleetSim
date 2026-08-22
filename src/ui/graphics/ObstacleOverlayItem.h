#pragma once

#include <QGraphicsItem>
#include <QVector>

namespace fleetsim::ui {

class ObstacleOverlayItem : public QGraphicsItem {
public:
    explicit ObstacleOverlayItem(QGraphicsItem* parent = nullptr);

    void setOccupiedCells(const QVector<QPointF>& cell_centers, double cell_size_m);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget) override;

private:
    QVector<QPointF> cell_centers_;
    double cell_size_m_{0.1};
};

}  // namespace fleetsim::ui
