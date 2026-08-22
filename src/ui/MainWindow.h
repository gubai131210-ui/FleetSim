#pragma once

#include <QKeyEvent>
#include <QMainWindow>
#include <QTimer>
#include <QVector>

#include "domain/map/MapData.h"
#include "dialogs/SettingsDialog.h"
#include "dialogs/PlannerTrackerDialog.h"

namespace fleetsim::app {
class FleetUiCoordinator;
class MonitorBridge;
class ProjectManager;
class SimController;
}

namespace fleetsim::ui {
class MapEditorPanel;
class MapView;
class MonitorPanel;
class TaskPanel;
class VehicleInfoPanel;
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
    void bindEditorSignals();
    void bindMonitorBridge();
    void bindTaskPanel();
    void refreshTaskPanel();

    void handleNewProject();
    void handleOpenProject();
    void handleSaveProject();
    void handleSettings();
    void handlePlannerTracker();
    void syncSettingsFromScenario();
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
    void handleAddTaskRequest(double pickup_x, double pickup_y, double dropoff_x, double dropoff_y);

    MapView* map_view_{nullptr};
    MapEditorPanel* editor_panel_{nullptr};
    MonitorPanel* monitor_panel_{nullptr};
    TaskPanel* task_panel_{nullptr};
    VehicleInfoPanel* vehicle_info_panel_{nullptr};

    app::SimController* sim_controller_{nullptr};
    app::ProjectManager* project_manager_{nullptr};
    app::MonitorBridge* monitor_bridge_{nullptr};
    app::FleetUiCoordinator* fleet_coordinator_{nullptr};

    QTimer* simulation_timer_{nullptr};
    std::vector<domain::map::MapDocument> undo_stack_;
    int next_task_index_{0};
    SimulationSettings current_settings_;
    PlannerTrackerSettings planner_tracker_settings_;
};

}  // namespace fleetsim::ui
