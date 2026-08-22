#pragma once

#include <QMainWindow>
#include <QTimer>

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
    void setupDockPanels();
    void setupViewMenu();
    void setupSimulationLoop();
    void loadDemoScenario();
    void setupVehicleFromScenario();
    void setupObstacleOverlay();
    void bindEventBus();
    void handleGoalRequest(double x_m, double y_m);
    QString resolveAssetPath(const QString& relative_path) const;
    QString demoScenarioDirectory() const;

    MapView* map_view_{nullptr};
    VehicleGraphicsItem* vehicle_item_{nullptr};
    app::SimController* sim_controller_{nullptr};
    QTimer* simulation_timer_{nullptr};
    int pose_subscription_id_{0};
    int path_subscription_id_{0};
    int goal_subscription_id_{0};
};

}  // namespace fleetsim::ui
