#include "FleetUiCoordinator.h"

#include "domain/scenario/ScenarioLoader.h"
#include "graphics/PathGraphicsItem.h"
#include "graphics/VehicleGraphicsItem.h"
#include "map/MapScene.h"
#include "map/MapView.h"
#include "panels/VehicleInfoPanel.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>

#include <nlohmann/json.hpp>

namespace fleetsim::app {

FleetUiCoordinator::FleetUiCoordinator(SimController* controller,
                                         ui::MapView* map_view,
                                         ui::VehicleInfoPanel* vehicle_info_panel,
                                         QObject* parent)
    : QObject(parent)
    , controller_(controller)
    , map_view_(map_view)
    , vehicle_info_panel_(vehicle_info_panel)
{
}

void FleetUiCoordinator::clearFleetGraphics()
{
    if (map_view_ == nullptr || map_view_->mapScene() == nullptr) {
        return;
    }

    for (ui::VehicleGraphicsItem* item : vehicle_items_) {
        map_view_->mapScene()->vehicleLayer()->removeFromGroup(item);
        delete item;
    }
    vehicle_items_.clear();
    map_view_->mapScene()->clearPathLayer();

    for (ui::PathGraphicsItem* item : path_items_) {
        delete item;
    }
    path_items_.clear();
}

QColor FleetUiCoordinator::pathColorForIndex(int index) const
{
    static const QVector<QColor> kColors = {
        QColor(46, 160, 67),
        QColor(31, 111, 235),
        QColor(210, 153, 34),
        QColor(171, 76, 209),
    };
    return kColors.at(index % kColors.size());
}

QString FleetUiCoordinator::resolveAssetPath(const QString& relative_path) const
{
    const QDir app_dir(QCoreApplication::applicationDirPath());
    const QString candidate = app_dir.filePath(relative_path);
    if (QFile::exists(candidate)) {
        return candidate;
    }

    const QDir source_dir(QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("../..")));
    return source_dir.filePath(relative_path);
}

void FleetUiCoordinator::rebuildFromScenario(const QString& /*assets_root*/)
{
    clearFleetGraphics();
    if (controller_ == nullptr || controller_->scenario() == nullptr || map_view_ == nullptr) {
        return;
    }

    const auto* scenario = controller_->scenario();
    int index = 0;
    for (const auto& vehicle_config : scenario->vehicles) {
        const QString svg_path = resolveAssetPath(QString::fromStdString(vehicle_config.svg_path));
        auto* vehicle_item = new ui::VehicleGraphicsItem(vehicle_config.id, svg_path, nullptr);
        vehicle_item->setVehicleLengthM(vehicle_config.length_m);

        const auto* vehicle = controller_->engine().fleet().findAgent(vehicle_config.id);
        if (vehicle != nullptr && vehicle->vehicle != nullptr) {
            vehicle_item->setPose(vehicle->vehicle->pose());
        }

        map_view_->mapScene()->vehicleLayer()->addToGroup(vehicle_item);
        vehicle_items_.insert(QString::fromStdString(vehicle_config.id), vehicle_item);

        auto* path_item = new ui::PathGraphicsItem();
        path_item->setPathColor(pathColorForIndex(index));
        map_view_->mapScene()->pathLayer()->addToGroup(path_item);
        path_items_.insert(QString::fromStdString(vehicle_config.id), path_item);

        connect(vehicle_item, &ui::VehicleGraphicsItem::selected, this, [this](const core::VehicleId& id) {
            setSelectedVehicle(QString::fromStdString(id));
            emit vehicleSelected(QString::fromStdString(id));
        });

        ++index;
    }
}

void FleetUiCoordinator::setSelectedVehicle(const QString& vehicle_id)
{
    if (controller_ != nullptr) {
        controller_->selectVehicle(vehicle_id.toStdString());
    }
    if (vehicle_info_panel_ != nullptr) {
        vehicle_info_panel_->setSelectedVehicleId(vehicle_id);
    }
}

void FleetUiCoordinator::bind()
{
    if (controller_ == nullptr || pose_subscription_id_ != 0) {
        return;
    }

    auto& bus = controller_->engine().eventBus();

    pose_subscription_id_ = bus.subscribe("sim/pose_updated", [this](const std::string& payload) {
        const nlohmann::json json = nlohmann::json::parse(payload);
        const QString id = QString::fromStdString(json.at("id").get<std::string>());
        auto* item = vehicle_items_.value(id, nullptr);
        if (item == nullptr) {
            return;
        }

        core::Pose pose;
        pose.x = json.at("x").get<double>();
        pose.y = json.at("y").get<double>();
        pose.theta = json.at("theta").get<double>();
        item->setPose(pose);

        if (vehicle_info_panel_ != nullptr
            && vehicle_info_panel_->selectedVehicleId() == id) {
            vehicle_info_panel_->updatePose(pose);
            vehicle_info_panel_->updateLinearVelocity(json.value("linear_velocity", 0.0));
            vehicle_info_panel_->updateTaskId(QString::fromStdString(json.value("task_id", "")));
        }
    });

    path_subscription_id_ = bus.subscribe("sim/path_updated", [this](const std::string& payload) {
        const nlohmann::json json = nlohmann::json::parse(payload);
        const QString id = QString::fromStdString(json.at("id").get<std::string>());
        auto* item = path_items_.value(id, nullptr);
        if (item == nullptr) {
            if (path_items_.size() == 1) {
                item = path_items_.begin().value();
            } else {
                return;
            }
        }

        QVector<QPointF> points;
        for (const auto& waypoint : json.at("waypoints")) {
            points.append(QPointF(waypoint.at("x").get<double>(), waypoint.at("y").get<double>()));
        }
        item->setPathPoints(points);
    });

    goal_subscription_id_ = bus.subscribe("sim/goal_reached", [this](const std::string& vehicle_id) {
        controller_->pause();
        emit fleetStatusMessage(
            tr("Goal reached for %1").arg(QString::fromStdString(vehicle_id)));
    });

    task_completed_subscription_id_ = bus.subscribe("sim/task_completed", [this](const std::string& task_id) {
        emit fleetStatusMessage(tr("Task completed: %1").arg(QString::fromStdString(task_id)));
    });
}

void FleetUiCoordinator::unbind()
{
    if (controller_ == nullptr) {
        return;
    }

    auto& bus = controller_->engine().eventBus();
    if (pose_subscription_id_ != 0) {
        bus.unsubscribe("sim/pose_updated", pose_subscription_id_);
        pose_subscription_id_ = 0;
    }
    if (path_subscription_id_ != 0) {
        bus.unsubscribe("sim/path_updated", path_subscription_id_);
        path_subscription_id_ = 0;
    }
    if (goal_subscription_id_ != 0) {
        bus.unsubscribe("sim/goal_reached", goal_subscription_id_);
        goal_subscription_id_ = 0;
    }
    if (task_completed_subscription_id_ != 0) {
        bus.unsubscribe("sim/task_completed", task_completed_subscription_id_);
        task_completed_subscription_id_ = 0;
    }
}

}  // namespace fleetsim::app
