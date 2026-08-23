#pragma once

#include <QWidget>
#include <QString>

class QComboBox;

namespace fleetsim::ui {

class ControlPage : public QWidget {
    Q_OBJECT

public:
    explicit ControlPage(QWidget* parent = nullptr);

    QString tracker() const;
    void setTracker(const QString& tracker);

private:
    QComboBox* tracker_combo_{nullptr};
};

}  // namespace fleetsim::ui
