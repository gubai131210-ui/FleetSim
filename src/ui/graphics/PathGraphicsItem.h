#pragma once

#include <QColor>
#include <QGraphicsItem>
#include <QVector>

namespace fleetsim::ui {

class PathGraphicsItem : public QGraphicsItem {
public:
    explicit PathGraphicsItem(QGraphicsItem* parent = nullptr);

    void setPathPoints(const QVector<QPointF>& points);
    void setPathColor(const QColor& color);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget) override;

private:
    QVector<QPointF> points_;
    QColor path_color_{QColor(46, 160, 67)};
};

}  // namespace fleetsim::ui
