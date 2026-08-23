#include "BehaviorTreePanel.h"

#include <QFormLayout>
#include <QLabel>

namespace fleetsim::ui {

BehaviorTreePanel::BehaviorTreePanel(QWidget* parent)
    : QWidget(parent)
{
    auto* form = new QFormLayout(this);
    mode_label_ = new QLabel(tr("legacy"), this);
    tree_label_ = new QLabel(tr("—"), this);
    active_label_ = new QLabel(tr("—"), this);
    status_label_ = new QLabel(tr("—"), this);
    path_valid_label_ = new QLabel(tr("—"), this);
    replan_label_ = new QLabel(tr("—"), this);
    recovery_label_ = new QLabel(tr("0"), this);

    form->addRow(tr("Behavior mode"), mode_label_);
    form->addRow(tr("Tree"), tree_label_);
    form->addRow(tr("Active node"), active_label_);
    form->addRow(tr("BT status"), status_label_);
    form->addRow(tr("path_valid"), path_valid_label_);
    form->addRow(tr("replan_requested"), replan_label_);
    form->addRow(tr("recovery_count"), recovery_label_);
}

void BehaviorTreePanel::updateStatus(const QString& behavior_mode,
                                     const QString& tree_name,
                                     const QString& active_node,
                                     const QString& node_status,
                                     bool path_valid,
                                     bool replan_requested,
                                     int recovery_count)
{
    mode_label_->setText(behavior_mode.isEmpty() ? tr("legacy") : behavior_mode);
    tree_label_->setText(tree_name.isEmpty() ? tr("—") : tree_name);
    active_label_->setText(active_node.isEmpty() ? tr("—") : active_node);
    status_label_->setText(node_status.isEmpty() ? tr("—") : node_status);
    path_valid_label_->setText(path_valid ? tr("true") : tr("false"));
    replan_label_->setText(replan_requested ? tr("true") : tr("false"));
    recovery_label_->setText(QString::number(recovery_count));
}

}  // namespace fleetsim::ui
