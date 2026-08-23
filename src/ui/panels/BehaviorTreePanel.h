#pragma once

#include <QWidget>
#include <QString>

class QLabel;

namespace fleetsim::ui {

/// Read-only BT status monitor dock (ADR-021). No tree editing.
class BehaviorTreePanel : public QWidget {
    Q_OBJECT

public:
    explicit BehaviorTreePanel(QWidget* parent = nullptr);

public slots:
    void updateStatus(const QString& behavior_mode,
                      const QString& tree_name,
                      const QString& active_node,
                      const QString& node_status,
                      bool path_valid,
                      bool replan_requested,
                      int recovery_count);

private:
    QLabel* mode_label_{nullptr};
    QLabel* tree_label_{nullptr};
    QLabel* active_label_{nullptr};
    QLabel* status_label_{nullptr};
    QLabel* path_valid_label_{nullptr};
    QLabel* replan_label_{nullptr};
    QLabel* recovery_label_{nullptr};
};

}  // namespace fleetsim::ui
