#pragma once

#include <QWidget>
#include <QString>

class QComboBox;
class QDoubleSpinBox;

namespace fleetsim::ui {

class RoutingPage : public QWidget {
    Q_OBJECT

public:
    explicit RoutingPage(QWidget* parent = nullptr);

    QString routingMode() const;
    double laneSnapRadiusM() const;
    QString firstLastPlanner() const;

    void setRoutingMode(const QString& mode);
    void setLaneSnapRadiusM(double radius_m);
    void setFirstLastPlanner(const QString& planner);

private:
    QComboBox* routing_mode_combo_{nullptr};
    QDoubleSpinBox* snap_radius_spin_{nullptr};
    QComboBox* first_last_planner_combo_{nullptr};
};

}  // namespace fleetsim::ui
