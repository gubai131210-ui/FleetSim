#pragma once

#include <QGraphicsView>

namespace fleetsim::ui {

class MapScene;

class MapView : public QGraphicsView {
    Q_OBJECT

public:
    explicit MapView(QWidget* parent = nullptr);

    MapScene* mapScene();

    void setMapSizeM(double width_m, double height_m);

signals:
    void goalRequested(double x_m, double y_m);

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    MapScene* scene_{nullptr};
};

}  // namespace fleetsim::ui
