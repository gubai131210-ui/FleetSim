#pragma once

#include <QGraphicsScene>

namespace fleetsim::ui {

/// Manages map layers: background, grid, obstacles, paths, vehicles.
class MapScene : public QGraphicsScene {
    Q_OBJECT

public:
    explicit MapScene(QObject* parent = nullptr);

    void setMapSizeM(double width_m, double height_m);
    double mapWidthM() const;
    double mapHeightM() const;

private:
    void rebuildBackground();

    double map_width_m_{20.0};
    double map_height_m_{15.0};
};

}  // namespace fleetsim::ui
