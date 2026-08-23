#pragma once

#include <QDialog>
#include <QString>

class QComboBox;

namespace fleetsim::ui {

struct PlannerTrackerSettings {
    QString planner{"auto"};          // auto | astar | hybrid_astar
    QString tracker{"auto"};          // auto | pure_pursuit | stanley | mpc
    QString coordination{"priority"}; // priority | none
    QString speed_planner{"none"};    // none | st_graph
};

class PlannerTrackerDialog : public QDialog {
    Q_OBJECT

public:
    explicit PlannerTrackerDialog(QWidget* parent = nullptr);

    PlannerTrackerSettings settings() const;
    void setSettings(const PlannerTrackerSettings& settings);

private:
    QComboBox* planner_combo_{nullptr};
    QComboBox* tracker_combo_{nullptr};
    QComboBox* coordination_combo_{nullptr};
    QComboBox* speed_planner_combo_{nullptr};
};

}  // namespace fleetsim::ui
