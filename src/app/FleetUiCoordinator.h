#pragma once

#include "SimController.h"

#include <QObject>
#include <QHash>
#include <QVector>

namespace fleetsim::ui {
class MapView;
class PathGraphicsItem;
class VehicleGraphicsItem;
class VehicleInfoPanel;
}

namespace fleetsim::app {

class FleetUiCoordinator : public QObject {
    Q_OBJECT

public:
    FleetUiCoordinator(SimController* controller,
                       ui::MapView* map_view,
                       ui::VehicleInfoPanel* vehicle_info_panel,
                       QObject* parent = nullptr);

    void rebuildFromScenario(const QString& assets_root);
    void bind();
    void unbind();

    void setSelectedVehicle(const QString& vehicle_id);

signals:
    void vehicleSelected(const QString& vehicle_id);
    void fleetStatusMessage(const QString& message);

private:
    void clearFleetGraphics();
    QColor pathColorForIndex(int index) const;
    QString resolveAssetPath(const QString& relative_path) const;

    SimController* controller_{nullptr};
    ui::MapView* map_view_{nullptr};
    ui::VehicleInfoPanel* vehicle_info_panel_{nullptr};

    QHash<QString, ui::VehicleGraphicsItem*> vehicle_items_;
    QHash<QString, ui::PathGraphicsItem*> path_items_;
    int pose_subscription_id_{0};
    int path_subscription_id_{0};
    int goal_subscription_id_{0};
    int task_completed_subscription_id_{0};
};

}  // namespace fleetsim::app
