#include "VehicleGraphicsItem.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QSvgRenderer>

namespace fleetsim::ui {

namespace {
constexpr double kPi = 3.14159265358979323846;
}  // namespace

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
    setPos(pose_.x, pose_.y);
}

VehicleGraphicsItem::~VehicleGraphicsItem() = default;

core::VehicleId VehicleGraphicsItem::vehicleId() const
{
    return id_;
}

void VehicleGraphicsItem::setPose(const core::Pose& pose)
{
    pose_ = pose;
    setPos(pose_.x, pose_.y);
    setRotation(pose_.theta * 180.0 / kPi);
}

core::Pose VehicleGraphicsItem::pose() const
{
    return pose_;
}

void VehicleGraphicsItem::setVehicleLengthM(double length_m)
{
    vehicle_length_m_ = length_m;
    update();
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
    Q_UNUSED(option)
    Q_UNUSED(widget)

    if (use_svg_ && svg_renderer_ != nullptr) {
        drawSvg(painter);
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
    use_svg_ = svg_renderer_->isValid();
    if (!use_svg_) {
        svg_renderer_.reset();
    }
}

void VehicleGraphicsItem::drawSvg(QPainter* painter)
{
    painter->setRenderHint(QPainter::Antialiasing, true);

    const double scale = vehicle_length_m_ / 100.0;
    painter->save();
    painter->scale(scale, scale);
    painter->translate(-50.0, -30.0);
    svg_renderer_->render(painter);
    painter->restore();
}

void VehicleGraphicsItem::drawFallback(QPainter* painter)
{
    painter->setRenderHint(QPainter::Antialiasing, true);

    const double half_len = vehicle_length_m_ * 0.5;
    const double half_wid = vehicle_length_m_ * 0.3;

    painter->setPen(QPen(Qt::darkBlue, 0.05));
    painter->setBrush(QColor(70, 130, 220, 200));
    painter->drawRect(QRectF(-half_len, -half_wid, vehicle_length_m_, half_wid * 2.0));

    painter->setBrush(Qt::yellow);
    QPolygonF arrow;
    arrow << QPointF(half_len, 0.0)
          << QPointF(half_len - 0.2, 0.1)
          << QPointF(half_len - 0.2, -0.1);
    painter->drawPolygon(arrow);
}

}  // namespace fleetsim::ui
