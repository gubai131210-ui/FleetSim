#pragma once

#include <QMainWindow>

namespace fleetsim::app {
class SimController;
}

namespace fleetsim::ui {
class MapView;
class VehicleGraphicsItem;
}

namespace fleetsim::ui {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private:
    void setupUiLayout();
    void setupDemoVehicle();
    QString resolveAssetPath(const QString& relative_path) const;

    MapView* map_view_{nullptr};
    VehicleGraphicsItem* demo_vehicle_{nullptr};
    app::SimController* sim_controller_{nullptr};
};

}  // namespace fleetsim::ui
