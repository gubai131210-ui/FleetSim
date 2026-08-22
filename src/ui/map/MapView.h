#pragma once

#include <QGraphicsView>

namespace fleetsim::ui {

class MapScene;

/// QGraphicsView wrapper: zoom, pan, simulation canvas.
/// Phase 1: grid + path + vehicle layers.
class MapView : public QGraphicsView {
    Q_OBJECT

public:
    explicit MapView(QWidget* parent = nullptr);

    MapScene* mapScene();

    void setMapSizeM(double width_m, double height_m);

protected:
    void wheelEvent(QWheelEvent* event) override;

private:
    MapScene* scene_{nullptr};
};

}  // namespace fleetsim::ui
