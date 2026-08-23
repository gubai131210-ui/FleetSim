#pragma once

#include <QWidget>
#include <QString>

class QComboBox;

namespace fleetsim::ui {

class SpeedPage : public QWidget {
    Q_OBJECT

public:
    explicit SpeedPage(QWidget* parent = nullptr);

    QString speedPlanner() const;
    void setSpeedPlanner(const QString& speed_planner);

    QString prediction() const;
    void setPrediction(const QString& prediction);

private:
    QComboBox* speed_planner_combo_{nullptr};
    QComboBox* prediction_combo_{nullptr};
};

}  // namespace fleetsim::ui
