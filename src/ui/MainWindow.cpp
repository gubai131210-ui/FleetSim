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
#include "dialogs/PlannerTrackerDialog.h"
#include "domain/scheduling/GreedyAssigner.h"
#include "domain/scheduling/HungarianAssigner.h"

#include "domain/scheduling/GreedyAssigner.h"
#include "domain/scheduling/HungarianAssigner.h"
#include "graphics/LaneGraphicsItem.h"
#include "graphics/ObstacleGraphicsItem.h"
#include "graphics/ObstacleOverlayItem.h"
#include "graphics/PathGraphicsItem.h"
#include "graphics/VehicleGraphicsItem.h"
#include "map/MapScene.h"
#include "map/MapView.h"
#include "panels/ControlPanel.h"
#include "panels/LaneEditorPanel.h"
#include "panels/MapEditorPanel.h"
#include "panels/MonitorPanel.h"
#include "panels/ExperimentComparePanel.h"
#include "panels/BehaviorTreePanel.h"
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

#include <algorithm>
#include <filesystem>
#include <nlohmann/json.hpp>

namespace fleetsim::ui {

namespace {

std::string resolveBehaviorTreePathForUi(const QString& configured_path,
                                         const QString& scenario_directory)
{
    if (configured_path.isEmpty()) {
        return {};
    }
    const std::filesystem::path configured(configured_path.toStdString());
    if (configured.is_absolute()) {
        return configured.string();
    }
    if (!scenario_directory.isEmpty()) {
        const std::filesystem::path in_scenario =
            std::filesystem::path(scenario_directory.toStdString()) / configured;
        if (std::filesystem::exists(in_scenario)) {
            return in_scenario.string();
        }
    }
    const std::filesystem::path in_assets =
        std::filesystem::path("assets") / "behavior_trees" / configured.filename();
    if (std::filesystem::exists(in_assets)) {
        return in_assets.string();
    }
    return configured.string();
}

}  // namespace

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
    bindLaneEditorSignals();
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
    connect(map_view_, &MapView::laneNodePlaceRequested, this, &MainWindow::handleLaneNodePlaceRequested);
}

void MainWindow::setupMenuBar()
{
    auto* file_menu = menuBar()->addMenu(tr("File"));
    file_menu->addAction(tr("New Project..."), this, &MainWindow::handleNewProject);
    file_menu->addAction(tr("Open Project..."), this, &MainWindow::handleOpenProject);
    file_menu->addAction(tr("Save Project"), this, &MainWindow::handleSaveProject);
    file_menu->addSeparator();
    file_menu->addAction(tr("Settings..."), this, &MainWindow::handleSettings);
    file_menu->addAction(tr("Algorithm Workbench..."), this, &MainWindow::handlePlannerTracker);
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

    auto* lane_editor_dock = new QDockWidget(tr("Lane Editor"), this);
    lane_editor_dock->setObjectName(QStringLiteral("LaneEditorDock"));
    lane_editor_panel_ = new LaneEditorPanel(lane_editor_dock);
    lane_editor_dock->setWidget(lane_editor_panel_);
    addDockWidget(Qt::LeftDockWidgetArea, lane_editor_dock);
    tabifyDockWidget(editor_dock, lane_editor_dock);

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

    auto* compare_dock = new QDockWidget(tr("Experiment Compare"), this);
    compare_dock->setObjectName(QStringLiteral("ExperimentCompareDock"));
    experiment_compare_panel_ = new ExperimentComparePanel(compare_dock);
    compare_dock->setWidget(experiment_compare_panel_);
    addDockWidget(Qt::RightDockWidgetArea, compare_dock);

    auto* behavior_dock = new QDockWidget(tr("Behavior Tree"), this);
    behavior_dock->setObjectName(QStringLiteral("BehaviorTreeDock"));
    behavior_tree_panel_ = new BehaviorTreePanel(behavior_dock);
    behavior_dock->setWidget(behavior_tree_panel_);
    addDockWidget(Qt::RightDockWidgetArea, behavior_dock);
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
    view_menu->addAction(tr("Lane Editor Panel"), this, [this]() {
        if (auto* dock = findChild<QDockWidget*>(QStringLiteral("LaneEditorDock"))) {
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
    view_menu->addAction(tr("Experiment Compare Panel"), this, [this]() {
        if (auto* dock = findChild<QDockWidget*>(QStringLiteral("ExperimentCompareDock"))) {
            dock->setVisible(!dock->isVisible());
        }
    });
    view_menu->addAction(tr("Behavior Tree Panel"), this, [this]() {
        if (auto* dock = findChild<QDockWidget*>(QStringLiteral("BehaviorTreeDock"))) {
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
    connect(editor_panel_, &MapEditorPanel::editModeToggled, this, [this](bool enabled) {
        if (enabled) {
            map_view_->setLaneEditModeEnabled(false);
        }
    });
}

void MainWindow::bindLaneEditorSignals()
{
    connect(lane_editor_panel_, &LaneEditorPanel::laneEditModeToggled, map_view_, &MapView::setLaneEditModeEnabled);
    connect(lane_editor_panel_, &LaneEditorPanel::toolChanged, this, [this](LaneEditorTool tool) {
        map_view_->setPlacingLaneNode(tool == LaneEditorTool::AddNode);
    });
    connect(lane_editor_panel_, &LaneEditorPanel::laneEditModeToggled, this, [this](bool enabled) {
        if (enabled) {
            map_view_->setEditModeEnabled(false);
        }
    });
    connect(lane_editor_panel_, &LaneEditorPanel::nodeSelected, this, [this](const QString& node_id) {
        map_view_->mapScene()->laneGraphicsItem()->setSelectedNodeId(node_id);
    });
    connect(lane_editor_panel_, &LaneEditorPanel::connectEdgeRequested,
            this, &MainWindow::handleLaneConnectEdge);
    connect(lane_editor_panel_, &LaneEditorPanel::deleteNodeRequested,
            this, &MainWindow::handleLaneDeleteNode);
    connect(lane_editor_panel_, &LaneEditorPanel::deleteEdgeRequested,
            this, &MainWindow::handleLaneDeleteEdge);
}

void MainWindow::bindMonitorBridge()
{
    connect(monitor_bridge_, &app::MonitorBridge::sampleReady, monitor_panel_, &MonitorPanel::appendSample);
    connect(monitor_bridge_, &app::MonitorBridge::experimentMetricsUpdated, experiment_compare_panel_,
            &ExperimentComparePanel::updateCurrentRun);
    connect(monitor_bridge_, &app::MonitorBridge::experimentBaselineUpdated, experiment_compare_panel_,
            &ExperimentComparePanel::updateBaseline);
    connect(monitor_bridge_, &app::MonitorBridge::behaviorTreeStatusUpdated, behavior_tree_panel_,
            &BehaviorTreePanel::updateStatus);
    connect(experiment_compare_panel_, &ExperimentComparePanel::captureBaselineRequested, monitor_bridge_,
            &app::MonitorBridge::captureBaseline);
    connect(experiment_compare_panel_, &ExperimentComparePanel::exportCsvRequested, this,
            [this](const QString& path) {
                if (monitor_bridge_->exportCurrentMetricsCsv(path.toStdString())) {
                    statusBar()->showMessage(tr("Exported metrics CSV: %1").arg(path));
                } else {
                    statusBar()->showMessage(tr("Failed to export metrics CSV."));
                }
            });
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

void MainWindow::handlePlannerTracker()
{
    PlannerTrackerDialog dialog(this);
    dialog.setSettings(planner_tracker_settings_);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    planner_tracker_settings_ = dialog.settings();
    sim_controller_->engine().setPlannerKind(planner_tracker_settings_.planner.toStdString());
    sim_controller_->engine().setTrackerKind(planner_tracker_settings_.tracker.toStdString());
    sim_controller_->engine().setCoordinationKind(
        planner_tracker_settings_.coordination.toStdString());
    sim_controller_->engine().setSpeedPlannerKind(
        planner_tracker_settings_.speed_planner.toStdString());
    sim_controller_->engine().setPredictionKind(
        planner_tracker_settings_.prediction.toStdString());
    sim_controller_->engine().setRoutingMode(planner_tracker_settings_.routing_mode.toStdString());
    sim_controller_->engine().setLaneSnapRadiusM(planner_tracker_settings_.lane_snap_radius_m);
    sim_controller_->engine().setFirstLastPlannerKind(
        planner_tracker_settings_.first_last_planner.toStdString());
    sim_controller_->engine().setBehaviorMode(
        planner_tracker_settings_.behavior_mode.toStdString());
    sim_controller_->engine().setReplanHz(planner_tracker_settings_.replan_hz);
    sim_controller_->engine().setRecoveryWaitTicks(
        planner_tracker_settings_.recovery_wait_ticks);

    if (sim_controller_->engine().behaviorMode() == "bt") {
        const QString scenario_dir =
            project_manager_->hasProject()
                ? QString::fromStdString(project_manager_->scenarioData().scenario_directory)
                : QString();
        const std::string tree_path = resolveBehaviorTreePathForUi(
            planner_tracker_settings_.behavior_tree_path, scenario_dir);
        if (!tree_path.empty()) {
            sim_controller_->engine().loadBehaviorTree(tree_path);
        }
    }

    if (project_manager_->hasProject()) {
        auto& scenario = project_manager_->scenarioData();
        scenario.simulation.planner = planner_tracker_settings_.planner.toStdString();
        scenario.simulation.tracker = planner_tracker_settings_.tracker.toStdString();
        scenario.simulation.coordination = planner_tracker_settings_.coordination.toStdString();
        scenario.simulation.speed_planner =
            planner_tracker_settings_.speed_planner.toStdString();
        scenario.simulation.prediction = planner_tracker_settings_.prediction.toStdString();
        scenario.simulation.routing_mode = planner_tracker_settings_.routing_mode.toStdString();
        scenario.simulation.lane_snap_radius_m = planner_tracker_settings_.lane_snap_radius_m;
        scenario.simulation.first_last_planner =
            planner_tracker_settings_.first_last_planner.toStdString();
        scenario.simulation.behavior_mode =
            planner_tracker_settings_.behavior_mode.toStdString();
        scenario.simulation.behavior_tree_path =
            planner_tracker_settings_.behavior_tree_path.toStdString();
        scenario.simulation.replan_hz = planner_tracker_settings_.replan_hz;
        scenario.simulation.recovery_wait_ticks =
            planner_tracker_settings_.recovery_wait_ticks;
    }

    statusBar()->showMessage(
        tr("Algorithm workbench applied (planner=%1, tracker=%2, behavior=%3).")
            .arg(planner_tracker_settings_.planner,
                 planner_tracker_settings_.tracker,
                 planner_tracker_settings_.behavior_mode));
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
    if (!scenario.simulation.planner.empty()) {
        planner_tracker_settings_.planner = QString::fromStdString(scenario.simulation.planner);
    }
    if (!scenario.simulation.tracker.empty()) {
        planner_tracker_settings_.tracker = QString::fromStdString(scenario.simulation.tracker);
    }
    if (!scenario.simulation.coordination.empty()) {
        planner_tracker_settings_.coordination =
            QString::fromStdString(scenario.simulation.coordination);
    }
    if (!scenario.simulation.speed_planner.empty()) {
        planner_tracker_settings_.speed_planner =
            QString::fromStdString(scenario.simulation.speed_planner);
    }
    if (!scenario.simulation.prediction.empty()) {
        planner_tracker_settings_.prediction =
            QString::fromStdString(scenario.simulation.prediction);
    }
    if (!scenario.simulation.routing_mode.empty()) {
        planner_tracker_settings_.routing_mode =
            QString::fromStdString(scenario.simulation.routing_mode);
    }
    planner_tracker_settings_.lane_snap_radius_m = scenario.simulation.lane_snap_radius_m;
    planner_tracker_settings_.first_last_planner =
        QString::fromStdString(scenario.simulation.first_last_planner);
    planner_tracker_settings_.behavior_mode =
        QString::fromStdString(scenario.simulation.behavior_mode);
    planner_tracker_settings_.behavior_tree_path =
        QString::fromStdString(scenario.simulation.behavior_tree_path);
    planner_tracker_settings_.replan_hz = scenario.simulation.replan_hz;
    planner_tracker_settings_.recovery_wait_ticks = scenario.simulation.recovery_wait_ticks;
    planner_tracker_settings_.recovery_enabled = scenario.simulation.recovery_wait_ticks > 0;
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
    scenario.lanes = project_manager_->mapDocument().lanes;
    scenario.scenario_directory = project_manager_->projectDirectory();

    sim_controller_->loadScenarioData(std::move(scenario));

    monitor_bridge_->reset();
    monitor_panel_->clearSamples();
    if (experiment_compare_panel_ != nullptr) {
        experiment_compare_panel_->clearBaseline();
    }

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
    refreshLaneOverlay();
    rebuildEditorObstacles();
}

void MainWindow::refreshLaneOverlay()
{
    domain::map::LaneMapData lanes;
    if (project_manager_->hasProject()) {
        lanes = project_manager_->mapDocument().lanes;
    }
    map_view_->mapScene()->laneGraphicsItem()->setLaneData(lanes);
    refreshLaneEditorPanel();
}

void MainWindow::refreshLaneEditorPanel()
{
    if (lane_editor_panel_ == nullptr) {
        return;
    }
    domain::map::LaneMapData lanes;
    if (project_manager_->hasProject()) {
        lanes = project_manager_->mapDocument().lanes;
    }
    lane_editor_panel_->refreshFromLaneData(lanes);
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

void MainWindow::handleLaneNodePlaceRequested(double x_m, double y_m)
{
    if (!project_manager_->hasProject()) {
        return;
    }
    pushUndoSnapshot();

    domain::map::LaneMapData& lanes = project_manager_->mapDocument().lanes;
    int max_index = -1;
    for (const domain::map::LaneNode& node : lanes.nodes) {
        if (node.id.size() > 1 && node.id.front() == 'n') {
            try {
                max_index = std::max(max_index, std::stoi(node.id.substr(1)));
            } catch (...) {
            }
        }
    }
    domain::map::LaneNode node;
    node.id = "n" + std::to_string(max_index + 1);
    node.x = x_m;
    node.y = y_m;
    lanes.nodes.push_back(std::move(node));
    applyProjectToSimulation();
    statusBar()->showMessage(tr("Added lane node %1").arg(QString::fromStdString(lanes.nodes.back().id)));
}

void MainWindow::handleLaneConnectEdge(const QString& from_id,
                                       const QString& to_id,
                                       bool bidirectional)
{
    if (!project_manager_->hasProject() || from_id.isEmpty() || to_id.isEmpty()) {
        return;
    }
    pushUndoSnapshot();

    domain::map::LaneEdge edge;
    edge.from = from_id.toStdString();
    edge.to = to_id.toStdString();
    edge.bidirectional = bidirectional;
    project_manager_->mapDocument().lanes.edges.push_back(std::move(edge));
    applyProjectToSimulation();
}

void MainWindow::handleLaneDeleteNode(const QString& node_id)
{
    if (!project_manager_->hasProject() || node_id.isEmpty()) {
        return;
    }
    pushUndoSnapshot();

    domain::map::LaneMapData& lanes = project_manager_->mapDocument().lanes;
    const std::string id = node_id.toStdString();
    lanes.nodes.erase(
        std::remove_if(lanes.nodes.begin(), lanes.nodes.end(),
                       [&id](const domain::map::LaneNode& node) { return node.id == id; }),
        lanes.nodes.end());
    lanes.edges.erase(
        std::remove_if(lanes.edges.begin(), lanes.edges.end(),
                       [&id](const domain::map::LaneEdge& edge) {
                           return edge.from == id || edge.to == id;
                       }),
        lanes.edges.end());
    applyProjectToSimulation();
}

void MainWindow::handleLaneDeleteEdge(int edge_index)
{
    if (!project_manager_->hasProject() || edge_index < 0) {
        return;
    }
    auto& edges = project_manager_->mapDocument().lanes.edges;
    if (edge_index >= static_cast<int>(edges.size())) {
        return;
    }
    pushUndoSnapshot();
    edges.erase(edges.begin() + edge_index);
    applyProjectToSimulation();
}

}  // namespace fleetsim::ui
