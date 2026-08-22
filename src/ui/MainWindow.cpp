#include "MainWindow.h"

#include "app/SimController.h"
#include "core/EventBus.h"
#include "core/types/Pose.h"
#include "domain/map/OccupancyGrid.h"
#include "graphics/ObstacleOverlayItem.h"
#include "graphics/PathGraphicsItem.h"
#include "graphics/VehicleGraphicsItem.h"
#include "map/MapScene.h"
#include "map/MapView.h"
#include "panels/ControlPanel.h"

#include <QCoreApplication>
#include <QDir>
#include <QDockWidget>
#include <QFile>
#include <QMenuBar>
#include <QStatusBar>
#include <QVector>

#include <nlohmann/json.hpp>

namespace fleetsim::ui {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , sim_controller_(new app::SimController())
    , simulation_timer_(new QTimer(this))
{
    setWindowTitle(tr("FleetSim — Multi-AGV Simulation"));
    resize(1200, 800);

    setupUiLayout();
    setupDockPanels();
    loadDemoScenario();
    setupVehicleFromScenario();
    setupObstacleOverlay();
    bindEventBus();
    setupSimulationLoop();

    statusBar()->showMessage(tr("Phase 1 — Shift+Click to set goal, then Play"));
}

MainWindow::~MainWindow()
{
    auto& bus = sim_controller_->engine().eventBus();
    bus.unsubscribe("sim/pose_updated", pose_subscription_id_);
    bus.unsubscribe("sim/path_updated", path_subscription_id_);
    bus.unsubscribe("sim/goal_reached", goal_subscription_id_);
}

void MainWindow::setupUiLayout()
{
    map_view_ = new MapView(this);
    setCentralWidget(map_view_);

    connect(map_view_, &MapView::goalRequested, this, &MainWindow::handleGoalRequest);
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
}

void MainWindow::setupViewMenu()
{
    auto* view_menu = menuBar()->addMenu(tr("View"));
    if (view_menu != nullptr) {
        view_menu->addAction(tr("Control Panel"), this, [this]() {
            if (auto* dock = findChild<QDockWidget*>(QStringLiteral("ControlDock"))) {
                dock->setVisible(!dock->isVisible());
            }
        });
    }
}

void MainWindow::setupSimulationLoop()
{
    setupViewMenu();
    simulation_timer_->setInterval(50);
    connect(simulation_timer_, &QTimer::timeout, this, [this]() {
        sim_controller_->tick();
    });
    simulation_timer_->start();
}

QString MainWindow::demoScenarioDirectory() const
{
    const QDir app_dir(QCoreApplication::applicationDirPath());
    const QString runtime_path = app_dir.filePath(QStringLiteral("assets/scenarios/demo"));
    if (QFile::exists(runtime_path + QStringLiteral("/map.json"))) {
        return runtime_path;
    }

    const QDir source_dir(QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("../..")));
    return source_dir.filePath(QStringLiteral("assets/scenarios/demo"));
}

void MainWindow::loadDemoScenario()
{
    const QString scenario_dir = demoScenarioDirectory();
    if (!sim_controller_->loadScenario(scenario_dir.toStdString())) {
        statusBar()->showMessage(tr("Failed to load demo scenario"));
        return;
    }

    const auto* scenario = sim_controller_->scenario();
    if (scenario != nullptr) {
        map_view_->setMapSizeM(scenario->map.widthM(), scenario->map.heightM());
    }
}

void MainWindow::setupVehicleFromScenario()
{
    const auto* scenario = sim_controller_->scenario();
    const auto* vehicle = sim_controller_->engine().vehicle();
    if (scenario == nullptr || scenario->vehicles.empty() || vehicle == nullptr) {
        return;
    }

    const auto& vehicle_config = scenario->vehicles.front();
    const QString svg_path = resolveAssetPath(QString::fromStdString(vehicle_config.svg_path));

    vehicle_item_ = new VehicleGraphicsItem(vehicle_config.id, svg_path, nullptr);
    vehicle_item_->setVehicleLengthM(vehicle_config.length_m);
    vehicle_item_->setPose(vehicle->pose());

    map_view_->mapScene()->vehicleLayer()->addToGroup(vehicle_item_);

    connect(vehicle_item_, &VehicleGraphicsItem::selected, this, [this](const core::VehicleId& id) {
        statusBar()->showMessage(tr("Selected vehicle: %1").arg(QString::fromStdString(id)));
    });
}

void MainWindow::setupObstacleOverlay()
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

    map_view_->mapScene()->obstacleOverlayItem()->setOccupiedCells(
        occupied_centers, grid.resolutionM());
}

void MainWindow::bindEventBus()
{
    auto& bus = sim_controller_->engine().eventBus();

    pose_subscription_id_ = bus.subscribe("sim/pose_updated", [this](const std::string& payload) {
        if (vehicle_item_ == nullptr) {
            return;
        }

        const nlohmann::json json = nlohmann::json::parse(payload);
        core::Pose pose;
        pose.x = json.at("x").get<double>();
        pose.y = json.at("y").get<double>();
        pose.theta = json.at("theta").get<double>();
        vehicle_item_->setPose(pose);
    });

    path_subscription_id_ = bus.subscribe("sim/path_updated", [this](const std::string& payload) {
        const nlohmann::json json = nlohmann::json::parse(payload);
        QVector<QPointF> points;
        for (const auto& waypoint : json) {
            points.append(QPointF(waypoint.at("x").get<double>(), waypoint.at("y").get<double>()));
        }
        map_view_->mapScene()->pathItem()->setPathPoints(points);
    });

    goal_subscription_id_ = bus.subscribe("sim/goal_reached", [this](const std::string& vehicle_id) {
        sim_controller_->pause();
        statusBar()->showMessage(
            tr("Goal reached for %1").arg(QString::fromStdString(vehicle_id)));
    });
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

QString MainWindow::resolveAssetPath(const QString& relative_path) const
{
    const QDir app_dir(QCoreApplication::applicationDirPath());
    const QString candidate = app_dir.filePath(relative_path);
    if (QFile::exists(candidate)) {
        return candidate;
    }

    const QDir source_dir(QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("../..")));
    return source_dir.filePath(relative_path);
}

}  // namespace fleetsim::ui
