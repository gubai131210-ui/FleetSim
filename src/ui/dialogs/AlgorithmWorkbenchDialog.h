#pragma once

#include <QDialog>
#include <QString>

class QTabWidget;

namespace fleetsim::ui {

class BehaviorPage;
class ControlPage;
class CoordinationPage;
class PlanningPage;
class RoutingPage;
class SpeedPage;

struct AlgorithmWorkbenchSettings {
    QString planner{"auto"};           // auto | astar | hybrid_astar
    QString tracker{"auto"};           // auto | pure_pursuit | stanley | mpc
    QString coordination{"priority"};  // priority | none
    QString speed_planner{"none"};     // none | st_graph
    QString prediction{"none"};        // none | constant_velocity
    QString routing_mode{"freespace"}; // freespace | lane_graph | hybrid
    double lane_snap_radius_m{1.0};
    QString first_last_planner;        // empty = same as Planning page
    QString behavior_mode{"legacy"};   // legacy | bt
    QString behavior_tree_path;
    double replan_hz{1.0};
    int recovery_wait_ticks{20};
    bool recovery_enabled{true};
};

class AlgorithmWorkbenchDialog : public QDialog {
    Q_OBJECT

public:
    explicit AlgorithmWorkbenchDialog(QWidget* parent = nullptr);

    AlgorithmWorkbenchSettings settings() const;
    void setSettings(const AlgorithmWorkbenchSettings& settings);

private:
    PlanningPage* planning_page_{nullptr};
    ControlPage* control_page_{nullptr};
    SpeedPage* speed_page_{nullptr};
    CoordinationPage* coordination_page_{nullptr};
    RoutingPage* routing_page_{nullptr};
    BehaviorPage* behavior_page_{nullptr};
    QTabWidget* tabs_{nullptr};
};

}  // namespace fleetsim::ui
