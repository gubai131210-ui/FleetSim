#include "MapView.h"
#include "MapScene.h"

#include <QWheelEvent>

namespace fleetsim::ui {

MapView::MapView(QWidget* parent)
    : QGraphicsView(parent)
    , scene_(new MapScene(this))
{
    setScene(scene_);
    setRenderHint(QPainter::Antialiasing, true);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
    fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
}

MapScene* MapView::mapScene()
{
    return scene_;
}

void MapView::setMapSizeM(double width_m, double height_m)
{
    scene_->setMapSizeM(width_m, height_m);
    fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
}

void MapView::wheelEvent(QWheelEvent* event)
{
    constexpr double kZoomFactor = 1.15;
    if (event->angleDelta().y() > 0) {
        scale(kZoomFactor, kZoomFactor);
    } else {
        scale(1.0 / kZoomFactor, 1.0 / kZoomFactor);
    }
}

}  // namespace fleetsim::ui
