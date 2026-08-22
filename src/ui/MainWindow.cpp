#include "MainWindow.h"

#include "app/FleetUiCoordinator.h"
#include "app/MonitorBridge.h"
#include "app/ProjectManager.h"
#include "app/SimController.h"
#include "core/EventBus.h"
#include "core/types/Task.h"
#include "domain/map/OccupancyGrid.h"
#include "dialogs/ProjectDialog.h"
#include "dialogs/SettingsDialog.h"
#include "domain/scheduling/GreedyAssigner.h"
#include "domain/scheduling/HungarianAssigner.h"

#include "domain/scheduling/GreedyAssigner.h"
#include "domain/scheduling/HungarianAssigner.h"
#include "graphics/ObstacleGraphicsItem.h"
#include "graphics/ObstacleOverlayItem.h"
#include "graphics/PathGraphicsItem.h"
#include "graphics/VehicleGraphicsItem.h"
#include "map/MapScene.h"
#include "map/MapView.h"
#include "panels/ControlPanel.h"
#include "panels/MapEditorPanel.h"
#include "panels/MonitorPanel.h"
#include "panels/TaskPanel.h"
#include "panels/VehicleInfoPanel.h"

#include <QCoreApplication>
#include <QDir>
#include <QDockWidget>
#include <QFile>
#include <QFileDialog>
#include <QKeyEvent>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>

#include <nlohmann/json.hpp>

namespace fleetsim::ui {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , sim_controller_(new app::SimController())
    , project_manager_(new app::ProjectManager())
    , monitor_bridge_(new app::MonitorBridge(sim_controller_, this))
    , simulation_timer_(new QTimer(this))
{
    setWindowTitle(tr("FleetSim — Multi-AGV Simulation"));
    resize(1280, 860);

    setupUiLayout();
    setupMenuBar();
    setupDockPanels();
    bindEditorSignals();
    bindMonitorBridge();
    bindTaskPanel();
    setupSimulationLoop();

    fleet_coordinator_ = new app::FleetUiCoordinator(
        sim_controller_, map_view_, vehicle_info_panel_, this);
    fleet_coordinator_->bind();
    connect(fleet_coordinator_, &app::FleetUiCoordinator::fleetStatusMessage, this, [this](const QString& message) {
        statusBar()->showMessage(message);
    });
    connect(fleet_coordinator_, &app::FleetUiCoordinator::vehicleSelected, this, [this](const QString& id) {
        statusBar()->showMessage(tr("Selected vehicle: %1").arg(id));
    });

    statusBar()->showMessage(tr("Phase 3 — File → Open to load a scenario (try multi_agv)"));
}

MainWindow::~MainWindow()
{
    if (fleet_coordinator_ != nullptr) {
        fleet_coordinator_->unbind();
    }
    monitor_bridge_->unbind();
}

void MainWindow::setupUiLayout()
{
    map_view_ = new MapView(this);
    setCentralWidget(map_view_);

    connect(map_view_, &MapView::goalRequested, this, &MainWindow::handleGoalRequest);
    connect(map_view_, &MapView::rectObstacleCreated, this, &MainWindow::handleRectObstacleCreated);
    connect(map_view_, &MapView::polygonObstacleCreated, this, &MainWindow::handlePolygonObstacleCreated);
    connect(map_view_, &MapView::startPointRequested, this, &MainWindow::handleStartPointRequested);
    connect(map_view_, &MapView::endPointRequested, this, &MainWindow::handleEndPointRequested);
}

void MainWindow::setupMenuBar()
{
    auto* file_menu = menuBar()->addMenu(tr("File"));
    file_menu->addAction(tr("New Project..."), this, &MainWindow::handleNewProject);
    file_menu->addAction(tr("Open Project..."), this, &MainWindow::handleOpenProject);
    file_menu->addAction(tr("Save Project"), this, &MainWindow::handleSaveProject);
    file_menu->addSeparator();
    file_menu->addAction(tr("Settings..."), this, &MainWindow::handleSettings);
    file_menu->addSeparator();
    file_menu->addAction(tr("Exit"), this, &QWidget::close);
}

void MainWindow::setupDockPanels()
{
    auto* control_dock = new QDockWidget(tr("Control"), this);
    control_dock->setObjectName(QStringLiteral("ControlDock"));
    control_dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    auto* control_panel = new ControlPanel(sim_controller_, control_dock);
    control_dock->setWidget(control_panel);
    addDockWidget(Qt::RightDockWidgetArea, control_dock);
    connect(control_panel, &ControlPanel::statusMessage, this, [this](const QString& message) {
        statusBar()->showMessage(message);
    });

    auto* editor_dock = new QDockWidget(tr("Editor"), this);
    editor_dock->setObjectName(QStringLiteral("EditorDock"));
    editor_panel_ = new MapEditorPanel(editor_dock);
    editor_dock->setWidget(editor_panel_);
    addDockWidget(Qt::LeftDockWidgetArea, editor_dock);

    auto* monitor_dock = new QDockWidget(tr("Monitor"), this);
    monitor_dock->setObjectName(QStringLiteral("MonitorDock"));
    monitor_panel_ = new MonitorPanel(monitor_dock);
    monitor_dock->setWidget(monitor_panel_);
    addDockWidget(Qt::BottomDockWidgetArea, monitor_dock);

    auto* task_dock = new QDockWidget(tr("Tasks"), this);
    task_dock->setObjectName(QStringLiteral("TaskDock"));
    task_panel_ = new TaskPanel(task_dock);
    task_dock->setWidget(task_panel_);
    addDockWidget(Qt::RightDockWidgetArea, task_dock);

    auto* vehicle_dock = new QDockWidget(tr("Vehicle Info"), this);
    vehicle_dock->setObjectName(QStringLiteral("VehicleInfoDock"));
    vehicle_info_panel_ = new VehicleInfoPanel(vehicle_dock);
    vehicle_dock->setWidget(vehicle_info_panel_);
    addDockWidget(Qt::RightDockWidgetArea, vehicle_dock);
    tabifyDockWidget(task_dock, vehicle_dock);
}

void MainWindow::setupViewMenu()
{
    auto* view_menu = menuBar()->addMenu(tr("View"));
    view_menu->addAction(tr("Control Panel"), this, [this]() {
        if (auto* dock = findChild<QDockWidget*>(QStringLiteral("ControlDock"))) {
            dock->setVisible(!dock->isVisible());
        }
    });
    view_menu->addAction(tr("Editor Panel"), this, [this]() {
        if (auto* dock = findChild<QDockWidget*>(QStringLiteral("EditorDock"))) {
            dock->setVisible(!dock->isVisible());
        }
    });
    view_menu->addAction(tr("Monitor Panel"), this, [this]() {
        if (auto* dock = findChild<QDockWidget*>(QStringLiteral("MonitorDock"))) {
            dock->setVisible(!dock->isVisible());
        }
    });
    view_menu->addAction(tr("Task Panel"), this, [this]() {
        if (auto* dock = findChild<QDockWidget*>(QStringLiteral("TaskDock"))) {
            dock->setVisible(!dock->isVisible());
        }
    });
    view_menu->addAction(tr("Vehicle Info Panel"), this, [this]() {
        if (auto* dock = findChild<QDockWidget*>(QStringLiteral("VehicleInfoDock"))) {
            dock->setVisible(!dock->isVisible());
        }
    });
}

void MainWindow::setupSimulationLoop()
{
    setupViewMenu();
    simulation_timer_->setInterval(50);
    connect(simulation_timer_, &QTimer::timeout, this, [this]() {
        sim_controller_->tick();
        refreshTaskPanel();
    });
    simulation_timer_->start();
}

void MainWindow::bindEditorSignals()
{
    connect(editor_panel_, &MapEditorPanel::editModeToggled, map_view_, &MapView::setEditModeEnabled);
    connect(editor_panel_, &MapEditorPanel::toolChanged, map_view_, &MapView::setEditorTool);
    connect(editor_panel_, &MapEditorPanel::undoRequested, this, &MainWindow::undoLastEdit);
}

void MainWindow::bindMonitorBridge()
{
    connect(monitor_bridge_, &app::MonitorBridge::sampleReady, monitor_panel_, &MonitorPanel::appendSample);
    monitor_bridge_->bind();
}

void MainWindow::bindTaskPanel()
{
    connect(task_panel_, &TaskPanel::addTaskRequested, this, &MainWindow::handleAddTaskRequest);
}

void MainWindow::refreshTaskPanel()
{
    if (task_panel_ == nullptr || !sim_controller_->scenario()) {
        return;
    }

    QStringList lines;
    const auto& tasks = sim_controller_->engine().scheduling().tasks().tasks();
    for (const core::Task& task : tasks) {
        QString status = tr("pending");
        if (task.status == core::TaskStatus::Assigned) {
            status = tr("assigned");
        } else if (task.status == core::TaskStatus::Done) {
            status = tr("done");
        }
        lines.append(QStringLiteral("%1 | (%2,%3) | %4")
                         .arg(QString::fromStdString(task.id))
                         .arg(task.pickup.x, 0, 'f', 1)
                         .arg(task.pickup.y, 0, 'f', 1)
                         .arg(status));
    }
    task_panel_->refreshTasks(lines);
}

void MainWindow::handleAddTaskRequest(double pickup_x, double pickup_y, double dropoff_x, double dropoff_y)
{
    core::Task task;
    task.id = "task_" + std::to_string(next_task_index_++);
    task.pickup = {pickup_x, pickup_y, 0.0};
    task.dropoff = {dropoff_x, dropoff_y, 0.0};
    task.status = core::TaskStatus::Pending;

    sim_controller_->addTask(task);
    if (project_manager_->hasProject()) {
        project_manager_->scenarioData().tasks.push_back(task);
    }
    refreshTaskPanel();
    statusBar()->showMessage(tr("Added task %1").arg(QString::fromStdString(task.id)));
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    if (event->matches(QKeySequence::Undo)) {
        undoLastEdit();
        event->accept();
        return;
    }
    QMainWindow::keyPressEvent(event);
}

void MainWindow::handleNewProject()
{
    ProjectDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted || dialog.selectedAction() != ProjectDialogAction::NewProject) {
        return;
    }

    const QString dir = dialog.projectDirectory();
    project_manager_->newProject(dir.toStdString());
    if (!project_manager_->save(dir.toStdString())) {
        QMessageBox::warning(this, tr("New Project"), tr("Failed to save new project files."));
        return;
    }

    undo_stack_.clear();
    applyProjectToSimulation();
    statusBar()->showMessage(tr("Created project: %1").arg(dir));
}

void MainWindow::handleOpenProject()
{
    const QString dir = QFileDialog::getExistingDirectory(this, tr("Open scenario directory"));
    if (dir.isEmpty()) {
        return;
    }

    if (!project_manager_->load(dir.toStdString())) {
        QMessageBox::warning(this, tr("Open Project"), tr("Failed to load project from %1").arg(dir));
        return;
    }

    undo_stack_.clear();
    syncSettingsFromScenario();
    applyProjectToSimulation();
    statusBar()->showMessage(tr("Opened project: %1").arg(dir));
}

void MainWindow::handleSaveProject()
{
    if (!project_manager_->hasProject()) {
        handleNewProject();
        return;
    }

    const QString dir = QString::fromStdString(project_manager_->projectDirectory());
    if (!project_manager_->save(dir.toStdString())) {
        QMessageBox::warning(this, tr("Save Project"), tr("Failed to save project."));
        return;
    }

    applyProjectToSimulation();
    statusBar()->showMessage(tr("Saved project: %1").arg(dir));
}

void MainWindow::handleSettings()
{
    SettingsDialog dialog(this);
    dialog.setSettings(current_settings_);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    current_settings_ = dialog.settings();

    if (current_settings_.assigner == QStringLiteral("hungarian")) {
        sim_controller_->engine().scheduling().setAssigner(
            std::make_unique<domain::scheduling::HungarianAssigner>());
    } else {
        sim_controller_->engine().scheduling().setAssigner(
            std::make_unique<domain::scheduling::GreedyAssigner>());
    }

    // Stamp model / bicycle params onto open project scenario, then recreate vehicles.
    if (project_manager_->hasProject()) {
        auto& scenario = project_manager_->scenarioData();
        for (auto& vehicle : scenario.vehicles) {
            vehicle.model = current_settings_.vehicle_model.toStdString();
            vehicle.wheelbase_m = current_settings_.wheelbase_m;
            vehicle.max_steering_rad = current_settings_.max_steering_rad;
        }
        applyProjectToSimulation();
    }

    statusBar()->showMessage(
        tr("Settings applied (model=%1, assigner=%2).")
            .arg(current_settings_.vehicle_model, current_settings_.assigner));
}

void MainWindow::syncSettingsFromScenario()
{
    if (!project_manager_->hasProject()) {
        return;
    }
    const auto& scenario = project_manager_->scenarioData();
    if (scenario.vehicles.empty()) {
        return;
    }
    // Reflect scenario file into Settings UI; do not overwrite scenario on load.
    const auto& vehicle = scenario.vehicles.front();
    current_settings_.vehicle_model = QString::fromStdString(vehicle.model);
    if (vehicle.model == "bicycle") {
        current_settings_.wheelbase_m = vehicle.wheelbase_m;
        current_settings_.max_steering_rad = vehicle.max_steering_rad;
    }
}

void MainWindow::applyProjectToSimulation()
{
    if (!project_manager_->hasProject()) {
        return;
    }

    // Preserve scenario vehicle models (e.g. bicycle_demo). Model overrides happen
    // only in handleSettings(), which stamps project data before calling this.
    auto scenario = project_manager_->scenarioData();
    scenario.map = project_manager_->buildOccupancyGrid(0.55);
    scenario.scenario_directory = project_manager_->projectDirectory();

    sim_controller_->loadScenarioData(std::move(scenario));

    monitor_bridge_->reset();
    monitor_panel_->clearSamples();

    const auto& map_doc = project_manager_->mapDocument();
    map_view_->setMapSizeM(map_doc.width_m, map_doc.height_m);
    refreshMapVisualization();

    if (fleet_coordinator_ != nullptr) {
        fleet_coordinator_->rebuildFromScenario(QString());
    }
    refreshTaskPanel();

    if (sim_controller_->scenario() != nullptr && !sim_controller_->scenario()->vehicles.empty()) {
        const QString first_id = QString::fromStdString(sim_controller_->scenario()->vehicles.front().id);
        if (fleet_coordinator_ != nullptr) {
            fleet_coordinator_->setSelectedVehicle(first_id);
        }
    }
}

void MainWindow::refreshMapVisualization()
{
    const auto& grid = sim_controller_->engine().map();
    if (grid.rows() == 0) {
        return;
    }

    QVector<QPointF> occupied_centers;
    for (int row = 0; row < grid.rows(); ++row) {
        for (int col = 0; col < grid.cols(); ++col) {
            if (grid.isOccupied(row, col)) {
                occupied_centers.append(QPointF(grid.cellCenterX(col), grid.cellCenterY(row)));
            }
        }
    }
    map_view_->mapScene()->obstacleOverlayItem()->setOccupiedCells(occupied_centers, grid.resolutionM());
    rebuildEditorObstacles();
}

void MainWindow::rebuildEditorObstacles()
{
    map_view_->mapScene()->clearEditorLayer();
    if (!project_manager_->hasProject()) {
        return;
    }

    for (const auto& obstacle : project_manager_->mapDocument().obstacles) {
        auto* item = new ObstacleGraphicsItem(obstacle);
        map_view_->mapScene()->editorLayer()->addToGroup(item);
    }
}

void MainWindow::pushUndoSnapshot()
{
    if (!project_manager_->hasProject()) {
        return;
    }
    undo_stack_.push_back(project_manager_->mapDocument());
    if (undo_stack_.size() > 50) {
        undo_stack_.erase(undo_stack_.begin());
    }
}

void MainWindow::undoLastEdit()
{
    if (undo_stack_.empty() || !project_manager_->hasProject()) {
        return;
    }
    project_manager_->mapDocument() = undo_stack_.back();
    undo_stack_.pop_back();
    applyProjectToSimulation();
    statusBar()->showMessage(tr("Undo applied"));
}

void MainWindow::handleGoalRequest(double x_m, double y_m)
{
    sim_controller_->setGoal(x_m, y_m, 0.0);
    if (sim_controller_->planPath()) {
        statusBar()->showMessage(tr("Path planned to (%1, %2)").arg(x_m, 0, 'f', 2).arg(y_m, 0, 'f', 2));
    } else {
        statusBar()->showMessage(tr("Failed to plan path to goal"));
    }
}

void MainWindow::handleRectObstacleCreated(double x, double y, double width, double height)
{
    if (!project_manager_->hasProject()) {
        return;
    }
    pushUndoSnapshot();

    domain::map::Obstacle obstacle;
    obstacle.type = domain::map::ObstacleType::Rect;
    obstacle.rect = {x, y, width, height};
    project_manager_->mapDocument().obstacles.push_back(obstacle);
    applyProjectToSimulation();
}

void MainWindow::handlePolygonObstacleCreated(const QVector<QPointF>& vertices)
{
    if (!project_manager_->hasProject()) {
        return;
    }
    pushUndoSnapshot();

    domain::map::Obstacle obstacle;
    obstacle.type = domain::map::ObstacleType::Polygon;
    for (const QPointF& point : vertices) {
        obstacle.polygon.vertices.emplace_back(point.x(), point.y());
    }
    project_manager_->mapDocument().obstacles.push_back(obstacle);
    applyProjectToSimulation();
}

void MainWindow::handleStartPointRequested(double x_m, double y_m)
{
    if (!project_manager_->hasProject() || project_manager_->scenarioData().vehicles.empty()) {
        return;
    }
    pushUndoSnapshot();
    project_manager_->scenarioData().vehicles.front().initial_pose.x = x_m;
    project_manager_->scenarioData().vehicles.front().initial_pose.y = y_m;
    applyProjectToSimulation();
}

void MainWindow::handleEndPointRequested(double x_m, double y_m)
{
    handleGoalRequest(x_m, y_m);
}

}  // namespace fleetsim::ui
