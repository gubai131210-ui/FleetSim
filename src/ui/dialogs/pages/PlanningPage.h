#pragma once

#include <QWidget>
#include <QString>

class QComboBox;

namespace fleetsim::ui {

class PlanningPage : public QWidget {
    Q_OBJECT

public:
    explicit PlanningPage(QWidget* parent = nullptr);

    QString planner() const;
    void setPlanner(const QString& planner);

private:
    QComboBox* planner_combo_{nullptr};
};

}  // namespace fleetsim::ui
