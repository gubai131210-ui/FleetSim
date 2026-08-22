#include "MapView.h"
#include "MapScene.h"

#include <QGraphicsPathItem>
#include <QGraphicsRectItem>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainterPath>
#include <QPen>
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

void MapView::setEditModeEnabled(bool enabled)
{
    edit_mode_enabled_ = enabled;
    setDragMode(enabled ? QGraphicsView::NoDrag : QGraphicsView::ScrollHandDrag);
    if (!enabled) {
        drawing_rect_ = false;
        polygon_points_.clear();
        if (preview_rect_ != nullptr) {
            scene()->removeItem(preview_rect_);
            delete preview_rect_;
            preview_rect_ = nullptr;
        }
        if (preview_polygon_ != nullptr) {
            scene()->removeItem(preview_polygon_);
            delete preview_polygon_;
            preview_polygon_ = nullptr;
        }
    }
}

void MapView::setEditorTool(EditorTool tool)
{
    editor_tool_ = tool;
    polygon_points_.clear();
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

void MapView::mousePressEvent(QMouseEvent* event)
{
    const QPointF scene_pos = mapToScene(event->pos());

    if (edit_mode_enabled_) {
        if (editor_tool_ == EditorTool::Rect && event->button() == Qt::LeftButton) {
            drawing_rect_ = true;
            rect_start_ = scene_pos;
            if (preview_rect_ == nullptr) {
                preview_rect_ = new QGraphicsRectItem();
                QPen pen(QColor(200, 80, 80), 0.05);
                pen.setStyle(Qt::DashLine);
                preview_rect_->setPen(pen);
                scene()->addItem(preview_rect_);
            }
            preview_rect_->setRect(QRectF(rect_start_, rect_start_));
            event->accept();
            return;
        }

        if (editor_tool_ == EditorTool::Polygon && event->button() == Qt::LeftButton) {
            polygon_points_.append(scene_pos);
            if (preview_polygon_ == nullptr) {
                preview_polygon_ = new QGraphicsPathItem();
                QPen pen(QColor(200, 80, 80), 0.05);
                pen.setStyle(Qt::DashLine);
                preview_polygon_->setPen(pen);
                scene()->addItem(preview_polygon_);
            }
            QPainterPath path;
            if (!polygon_points_.isEmpty()) {
                path.moveTo(polygon_points_.first());
                for (int i = 1; i < polygon_points_.size(); ++i) {
                    path.lineTo(polygon_points_.at(i));
                }
            }
            preview_polygon_->setPath(path);
            event->accept();
            return;
        }

        if (editor_tool_ == EditorTool::StartPoint && event->button() == Qt::LeftButton) {
            emit startPointRequested(scene_pos.x(), scene_pos.y());
            event->accept();
            return;
        }

        if (editor_tool_ == EditorTool::EndPoint && event->button() == Qt::LeftButton) {
            emit endPointRequested(scene_pos.x(), scene_pos.y());
            event->accept();
            return;
        }
    }

    if (!edit_mode_enabled_ && event->modifiers().testFlag(Qt::ShiftModifier)
        && event->button() == Qt::LeftButton) {
        emit goalRequested(scene_pos.x(), scene_pos.y());
        event->accept();
        return;
    }

    QGraphicsView::mousePressEvent(event);
}

void MapView::mouseMoveEvent(QMouseEvent* event)
{
    if (drawing_rect_ && preview_rect_ != nullptr) {
        const QPointF scene_pos = mapToScene(event->pos());
        const QRectF rect = QRectF(rect_start_, scene_pos).normalized();
        preview_rect_->setRect(rect);
        event->accept();
        return;
    }
    QGraphicsView::mouseMoveEvent(event);
}

void MapView::mouseReleaseEvent(QMouseEvent* event)
{
    if (drawing_rect_ && event->button() == Qt::LeftButton) {
        const QPointF scene_pos = mapToScene(event->pos());
        const QRectF rect = QRectF(rect_start_, scene_pos).normalized();
        drawing_rect_ = false;
        if (preview_rect_ != nullptr) {
            scene()->removeItem(preview_rect_);
            delete preview_rect_;
            preview_rect_ = nullptr;
        }
        if (rect.width() > 0.05 && rect.height() > 0.05) {
            emit rectObstacleCreated(rect.x(), rect.y(), rect.width(), rect.height());
        }
        event->accept();
        return;
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void MapView::keyPressEvent(QKeyEvent* event)
{
    if (edit_mode_enabled_ && editor_tool_ == EditorTool::Polygon && event->key() == Qt::Key_Return) {
        if (polygon_points_.size() >= 3) {
            emit polygonObstacleCreated(polygon_points_);
        }
        polygon_points_.clear();
        if (preview_polygon_ != nullptr) {
            scene()->removeItem(preview_polygon_);
            delete preview_polygon_;
            preview_polygon_ = nullptr;
        }
        event->accept();
        return;
    }
    QGraphicsView::keyPressEvent(event);
}

}  // namespace fleetsim::ui
