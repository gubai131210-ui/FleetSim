#pragma once

#include "domain/map/LaneTypes.h"

#include <QColor>
#include <QGraphicsItem>
#include <QString>

namespace fleetsim::ui {

class LaneGraphicsItem : public QGraphicsItem {
public:
    explicit LaneGraphicsItem(QGraphicsItem* parent = nullptr);

    void setLaneData(const domain::map::LaneMapData& lanes);
    void setSelectedNodeId(const QString& node_id);
    void clearSelection();

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget) override;

private:
    domain::map::LaneMapData lanes_;
    QString selected_node_id_;
    QColor edge_color_{QColor(37, 99, 235)};
    QColor edge_bidirectional_color_{QColor(59, 130, 246)};
    QColor node_color_{QColor(30, 64, 175)};
    QColor selected_node_color_{QColor(234, 88, 12)};
};

}  // namespace fleetsim::ui
