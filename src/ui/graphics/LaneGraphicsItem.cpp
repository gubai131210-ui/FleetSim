#include "LaneGraphicsItem.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

#include <cmath>
#include <optional>
#include <utility>

namespace fleetsim::ui {

namespace {

constexpr double kNodeRadiusM = 0.12;
constexpr double kEdgeWidthM = 0.06;
constexpr double kArrowLengthM = 0.18;

std::optional<QPointF> lookupNode(const domain::map::LaneMapData& lanes, const std::string& node_id)
{
    for (const domain::map::LaneNode& node : lanes.nodes) {
        if (node.id == node_id) {
            return QPointF(node.x, node.y);
        }
    }
    return std::nullopt;
}

void drawDirectedEdge(QPainter* painter, const QPointF& from, const QPointF& to)
{
    painter->drawLine(from, to);

    const QPointF delta = to - from;
    const double length = std::hypot(delta.x(), delta.y());
    if (length < 1e-6) {
        return;
    }

    const QPointF unit(delta.x() / length, delta.y() / length);
    const QPointF tip = from + unit * (length * 0.82);
    const QPointF ortho(-unit.y(), unit.x());
    const QPointF left = tip - unit * kArrowLengthM + ortho * (kArrowLengthM * 0.45);
    const QPointF right = tip - unit * kArrowLengthM - ortho * (kArrowLengthM * 0.45);

    painter->drawLine(tip, left);
    painter->drawLine(tip, right);
}

}  // namespace

LaneGraphicsItem::LaneGraphicsItem(QGraphicsItem* parent)
    : QGraphicsItem(parent)
{
    setZValue(12.0);
}

void LaneGraphicsItem::setLaneData(const domain::map::LaneMapData& lanes)
{
    prepareGeometryChange();
    lanes_ = lanes;
    update();
}

void LaneGraphicsItem::setSelectedNodeId(const QString& node_id)
{
    if (selected_node_id_ == node_id) {
        return;
    }
    selected_node_id_ = node_id;
    update();
}

void LaneGraphicsItem::clearSelection()
{
    setSelectedNodeId({});
}

QRectF LaneGraphicsItem::boundingRect() const
{
    if (lanes_.nodes.empty()) {
        return {};
    }

    QRectF bounds;
    for (const domain::map::LaneNode& node : lanes_.nodes) {
        bounds |= QRectF(node.x - kNodeRadiusM,
                         node.y - kNodeRadiusM,
                         kNodeRadiusM * 2.0,
                         kNodeRadiusM * 2.0);
    }
    return bounds.adjusted(-0.2, -0.2, 0.2, 0.2);
}

void LaneGraphicsItem::paint(QPainter* painter,
                             const QStyleOptionGraphicsItem* option,
                             QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    if (lanes_.nodes.empty()) {
        return;
    }

    painter->setRenderHint(QPainter::Antialiasing, true);

    for (const domain::map::LaneEdge& edge : lanes_.edges) {
        const auto from = lookupNode(lanes_, edge.from);
        const auto to = lookupNode(lanes_, edge.to);
        if (!from.has_value() || !to.has_value()) {
            continue;
        }

        QPen pen(edge.bidirectional ? edge_bidirectional_color_ : edge_color_, kEdgeWidthM);
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        if (edge.bidirectional) {
            pen.setStyle(Qt::DashLine);
        }
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);

        if (edge.bidirectional) {
            painter->drawLine(*from, *to);
        } else {
            drawDirectedEdge(painter, *from, *to);
        }
    }

    for (const domain::map::LaneNode& node : lanes_.nodes) {
        const bool selected = selected_node_id_ == QString::fromStdString(node.id);
        painter->setPen(QPen(selected ? selected_node_color_ : node_color_, 0.03));
        painter->setBrush(selected ? selected_node_color_ : node_color_);
        painter->drawEllipse(QPointF(node.x, node.y), kNodeRadiusM, kNodeRadiusM);
    }
}

}  // namespace fleetsim::ui
