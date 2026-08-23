#pragma once

#include <QWidget>
#include <QString>

class QComboBox;

namespace fleetsim::ui {

class CoordinationPage : public QWidget {
    Q_OBJECT

public:
    explicit CoordinationPage(QWidget* parent = nullptr);

    QString coordination() const;
    void setCoordination(const QString& coordination);

private:
    QComboBox* coordination_combo_{nullptr};
};

}  // namespace fleetsim::ui
