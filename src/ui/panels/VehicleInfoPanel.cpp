#include "VehicleInfoPanel.h"

#include <QLabel>
#include <QVBoxLayout>

namespace fleetsim::ui {

VehicleInfoPanel::VehicleInfoPanel(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    vehicle_id_label_ = new QLabel(tr("Vehicle: (none)"), this);
    pose_label_ = new QLabel(tr("Pose: -"), this);
    velocity_label_ = new QLabel(tr("Velocity: -"), this);
    task_label_ = new QLabel(tr("Task: -"), this);
    layout->addWidget(vehicle_id_label_);
    layout->addWidget(pose_label_);
    layout->addWidget(velocity_label_);
    layout->addWidget(task_label_);
    layout->addStretch();
}

void VehicleInfoPanel::setSelectedVehicleId(const QString& vehicle_id)
{
    selected_vehicle_id_ = vehicle_id;
    vehicle_id_label_->setText(tr("Vehicle: %1").arg(vehicle_id.isEmpty() ? tr("(none)") : vehicle_id));
}

void VehicleInfoPanel::updatePose(const core::Pose& pose)
{
    pose_label_->setText(
        tr("Pose: (%1, %2, %3°)")
            .arg(pose.x, 0, 'f', 2)
            .arg(pose.y, 0, 'f', 2)
            .arg(pose.theta * 57.2958, 0, 'f', 1));
}

void VehicleInfoPanel::updateLinearVelocity(double velocity_mps)
{
    velocity_label_->setText(tr("Velocity: %1 m/s").arg(velocity_mps, 0, 'f', 2));
}

void VehicleInfoPanel::updateTaskId(const QString& task_id)
{
    task_label_->setText(tr("Task: %1").arg(task_id.isEmpty() ? tr("-") : task_id));
}

}  // namespace fleetsim::ui
