#pragma once

#include "core/types/Pose.h"
#include "core/types/VehicleId.h"

#include <QGraphicsObject>

#include <memory>

class QSvgRenderer;

namespace fleetsim::ui {

/// Renders a single AGV from an SVG asset. See ADR-004.
class VehicleGraphicsItem : public QGraphicsObject {
    Q_OBJECT

public:
    explicit VehicleGraphicsItem(const core::VehicleId& id,
                                 const QString& svg_path,
                                 QGraphicsItem* parent = nullptr);

    core::VehicleId vehicleId() const;

    void setPose(const core::Pose& pose);
    core::Pose pose() const;

    void setVehicleLengthM(double length_m);

    QRectF boundingRect() const override;

    ~VehicleGraphicsItem() override;

signals:
    void selected(const fleetsim::core::VehicleId& id);

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    void loadSvgOrFallback(const QString& svg_path);
    void drawFallback(QPainter* painter);
    void drawSvg(QPainter* painter);

    core::VehicleId id_;
    core::Pose pose_;
    double vehicle_length_m_{1.0};
    std::unique_ptr<QSvgRenderer> svg_renderer_;
    bool use_svg_{false};
};

}  // namespace fleetsim::ui
