#include "MainWindow.h"

#include "app/SimController.h"
#include "graphics/VehicleGraphicsItem.h"
#include "map/MapScene.h"
#include "map/MapView.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QStatusBar>
#include <QToolBar>

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

    auto* toolbar = addToolBar(tr("Simulation"));
    toolbar->setMovable(false);

    // Phase 1: wire play/pause/step to SimController.
    toolbar->addAction(tr("Play"), this, [this]() {
        sim_controller_->start();
        statusBar()->showMessage(tr("Simulation running"));
    });
    toolbar->addAction(tr("Pause"), this, [this]() {
        sim_controller_->pause();
        statusBar()->showMessage(tr("Simulation paused"));
    });
    toolbar->addAction(tr("Step"), this, [this]() {
        sim_controller_->stepOnce();
        statusBar()->showMessage(
            tr("Tick count: %1").arg(sim_controller_->engine().tickCount()));
    });
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

    // Qt Creator may run from build dir; also try source tree (dev fallback).
    const QDir source_dir(QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("../..")));
    return source_dir.filePath(relative_path);
}

}  // namespace fleetsim::ui
