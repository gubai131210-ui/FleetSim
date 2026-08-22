#include "VehicleGraphicsItem.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QSvgRenderer>

#include <cmath>

namespace fleetsim::ui {

VehicleGraphicsItem::VehicleGraphicsItem(const core::VehicleId& id,
                                         const QString& svg_path,
                                         QGraphicsItem* parent)
    : QGraphicsObject(parent)
    , id_(id)
{
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setAcceptHoverEvents(true);
    loadSvgOrFallback(svg_path);
    updateTransform();
}

core::VehicleId VehicleGraphicsItem::vehicleId() const
{
    return id_;
}

void VehicleGraphicsItem::setPose(const core::Pose& pose)
{
    pose_ = pose;
    updateTransform();
}

core::Pose VehicleGraphicsItem::pose() const
{
    return pose_;
}

void VehicleGraphicsItem::setVehicleLengthM(double length_m)
{
    vehicle_length_m_ = length_m;
    updateTransform();
}

QRectF VehicleGraphicsItem::boundingRect() const
{
    const double half_len = vehicle_length_m_ * 0.5;
    const double half_wid = vehicle_length_m_ * 0.3;
    return QRectF(-half_len, -half_wid, vehicle_length_m_, vehicle_length_m_ * 0.6);
}

void VehicleGraphicsItem::paint(QPainter* painter,
                                const QStyleOptionGraphicsItem* option,
                                QWidget* widget)
{
    if (use_svg_ && svg_item_ != nullptr) {
        QGraphicsObject::paint(painter, option, widget);
        return;
    }
    drawFallback(painter);
}

void VehicleGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsObject::mousePressEvent(event);
    emit selected(id_);
}

void VehicleGraphicsItem::loadSvgOrFallback(const QString& svg_path)
{
    svg_renderer_ = std::make_unique<QSvgRenderer>(svg_path);
    if (svg_renderer_->isValid()) {
        svg_item_ = new QGraphicsSvgItem(svg_path, this);
        use_svg_ = true;
        return;
    }

    svg_renderer_.reset();
    svg_item_ = nullptr;
    use_svg_ = false;
}

void VehicleGraphicsItem::updateTransform()
{
    setPos(pose_.x, pose_.y);
    setRotation(pose_.theta * 180.0 / M_PI);

    if (svg_item_ != nullptr) {
        // SVG viewBox is 100x60 logical units; scale to vehicle_length_m.
        const double scale = vehicle_length_m_ / 100.0;
        svg_item_->setScale(scale);
        svg_item_->setTransformOriginPoint(50.0, 30.0);
        svg_item_->setPos(-50.0 * scale, -30.0 * scale);
    }
}

void VehicleGraphicsItem::drawFallback(QPainter* painter)
{
    painter->setRenderHint(QPainter::Antialiasing, true);

    const double half_len = vehicle_length_m_ * 0.5;
    const double half_wid = vehicle_length_m_ * 0.3;

    painter->setPen(QPen(Qt::darkBlue, 0.05));
    painter->setBrush(QColor(70, 130, 220, 200));
    painter->drawRect(QRectF(-half_len, -half_wid, vehicle_length_m_, half_wid * 2.0));

    // Forward direction indicator (+X)
    painter->setBrush(Qt::yellow);
    QPolygonF arrow;
    arrow << QPointF(half_len, 0.0)
          << QPointF(half_len - 0.2, 0.1)
          << QPointF(half_len - 0.2, -0.1);
    painter->drawPolygon(arrow);
}

}  // namespace fleetsim::ui
