#pragma once

#include <QWidget>

class QTableWidget;

namespace fleetsim::ui {

class TaskPanel : public QWidget {
    Q_OBJECT

public:
    explicit TaskPanel(QWidget* parent = nullptr);

    void refreshTasks(const QStringList& lines);

signals:
    void addTaskRequested(double pickup_x, double pickup_y, double dropoff_x, double dropoff_y);

private:
    QTableWidget* task_table_{nullptr};
};

}  // namespace fleetsim::ui
