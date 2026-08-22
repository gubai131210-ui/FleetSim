#include "MainWindow.h"

#include "app/SimController.h"
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

namespace fleetsim::ui {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , sim_controller_(new app::SimController())
{
    setWindowTitle(tr("FleetSim — Multi-AGV Simulation"));
    resize(1200, 800);

    setupUiLayout();
    setupDemoVehicle();

    statusBar()->showMessage(tr("Phase 0 — Project skeleton ready"));
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUiLayout()
{
    map_view_ = new MapView(this);
    map_view_->setMapSizeM(20.0, 15.0);
    setCentralWidget(map_view_);

    setupDockPanels();
    setupViewMenu();
}

void MainWindow::setupDockPanels()
{
    auto* control_dock = new QDockWidget(tr("Control"), this);
    control_dock->setObjectName(QStringLiteral("ControlDock"));
    control_dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    auto* control_panel = new ControlPanel(sim_controller_, control_dock);
    control_dock->setWidget(control_panel);
    addDockWidget(Qt::RightDockWidgetArea, control_dock);

    connect(control_panel, &ControlPanel::statusMessage,
            statusBar(), &QStatusBar::showMessage);
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

void MainWindow::setupDemoVehicle()
{
    const QString svg_path = resolveAssetPath(QStringLiteral("assets/vehicles/agv_diff.svg"));

    demo_vehicle_ = new VehicleGraphicsItem(
        core::VehicleId{"agv_0"},
        svg_path,
        nullptr);

    demo_vehicle_->setVehicleLengthM(1.0);
    demo_vehicle_->setPose(core::Pose{2.0, 2.0, 0.0});

    map_view_->mapScene()->addItem(demo_vehicle_);

    connect(demo_vehicle_, &VehicleGraphicsItem::selected, this, [this](const core::VehicleId& id) {
        statusBar()->showMessage(tr("Selected vehicle: %1").arg(QString::fromStdString(id)));
    });
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
