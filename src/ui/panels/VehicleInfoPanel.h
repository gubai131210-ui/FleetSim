#pragma once

#include "core/types/Pose.h"

#include <QWidget>

namespace fleetsim::ui {

class VehicleInfoPanel : public QWidget {
    Q_OBJECT

public:
    explicit VehicleInfoPanel(QWidget* parent = nullptr);

    QString selectedVehicleId() const { return selected_vehicle_id_; }

    void setSelectedVehicleId(const QString& vehicle_id);
    void updatePose(const core::Pose& pose);
    void updateLinearVelocity(double velocity_mps);
    void updateTaskId(const QString& task_id);

private:
    class QLabel* vehicle_id_label_{nullptr};
    class QLabel* pose_label_{nullptr};
    class QLabel* velocity_label_{nullptr};
    class QLabel* task_label_{nullptr};
    QString selected_vehicle_id_;
};

}  // namespace fleetsim::ui
