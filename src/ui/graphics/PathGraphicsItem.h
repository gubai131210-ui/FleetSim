#pragma once

#include <QGraphicsItem>
#include <QVector>

namespace fleetsim::ui {

class PathGraphicsItem : public QGraphicsItem {
public:
    explicit PathGraphicsItem(QGraphicsItem* parent = nullptr);

    void setPathPoints(const QVector<QPointF>& points);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget) override;

private:
    QVector<QPointF> points_;
};

}  // namespace fleetsim::ui
