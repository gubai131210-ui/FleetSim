#pragma once

#include <QKeyEvent>
#include <QMainWindow>
#include <QTimer>
#include <QVector>

#include "domain/map/MapData.h"

namespace fleetsim::app {
class MonitorBridge;
class ProjectManager;
class SimController;
}

namespace fleetsim::ui {
class MapEditorPanel;
class MapView;
class MonitorPanel;
class VehicleGraphicsItem;
}

namespace fleetsim::ui {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void keyPressEvent(QKeyEvent* event) override;

private:
    void setupUiLayout();
    void setupMenuBar();
    void setupDockPanels();
    void setupViewMenu();
    void setupSimulationLoop();
    void bindEventBus();
    void bindEditorSignals();
    void bindMonitorBridge();

    void handleNewProject();
    void handleOpenProject();
    void handleSaveProject();
    void applyProjectToSimulation();
    void refreshMapVisualization();
    void rebuildEditorObstacles();
    void pushUndoSnapshot();
    void undoLastEdit();

    void handleGoalRequest(double x_m, double y_m);
    void handleRectObstacleCreated(double x, double y, double width, double height);
    void handlePolygonObstacleCreated(const QVector<QPointF>& vertices);
    void handleStartPointRequested(double x_m, double y_m);
    void handleEndPointRequested(double x_m, double y_m);

    QString resolveAssetPath(const QString& relative_path) const;

    MapView* map_view_{nullptr};
    MapEditorPanel* editor_panel_{nullptr};
    MonitorPanel* monitor_panel_{nullptr};
    VehicleGraphicsItem* vehicle_item_{nullptr};

    app::SimController* sim_controller_{nullptr};
    app::ProjectManager* project_manager_{nullptr};
    app::MonitorBridge* monitor_bridge_{nullptr};

    QTimer* simulation_timer_{nullptr};
    int pose_subscription_id_{0};
    int path_subscription_id_{0};
    int goal_subscription_id_{0};

    std::vector<domain::map::MapDocument> undo_stack_;
};

}  // namespace fleetsim::ui
