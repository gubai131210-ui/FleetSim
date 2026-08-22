#include "MapScene.h"

#include <QBrush>
#include <QPen>

namespace fleetsim::ui {

MapScene::MapScene(QObject* parent)
    : QGraphicsScene(parent)
{
    rebuildBackground();
}

void MapScene::setMapSizeM(double width_m, double height_m)
{
    map_width_m_ = width_m;
    map_height_m_ = height_m;
    rebuildBackground();
}

double MapScene::mapWidthM() const
{
    return map_width_m_;
}

double MapScene::mapHeightM() const
{
    return map_height_m_;
}

void MapScene::rebuildBackground()
{
    clear();
    setSceneRect(0.0, 0.0, map_width_m_, map_height_m_);

    QPen border_pen(Qt::darkGray);
    border_pen.setWidthF(0.05);
    addRect(0.0, 0.0, map_width_m_, map_height_m_, border_pen, QBrush(Qt::white));

    // Light grid (1 m spacing) — Phase 1 will toggle via occupancy overlay.
    QPen grid_pen(QColor(220, 220, 220));
    grid_pen.setWidthF(0.01);
    for (double x = 0.0; x <= map_width_m_; x += 1.0) {
        addLine(x, 0.0, x, map_height_m_, grid_pen);
    }
    for (double y = 0.0; y <= map_height_m_; y += 1.0) {
        addLine(0.0, y, map_width_m_, y, grid_pen);
    }
}

}  // namespace fleetsim::ui
