#pragma once

#include <QWidget>

namespace fleetsim::app {
class SimController;
}

namespace fleetsim::ui {

/// Simulation transport controls — lives in its own panel, NOT in MainWindow toolbar.
/// Phase 1+: add speed multiplier, dt display here.
class ControlPanel : public QWidget {
    Q_OBJECT

public:
    explicit ControlPanel(app::SimController* controller, QWidget* parent = nullptr);

signals:
    void statusMessage(const QString& message);

private:
    void setupUi();

    app::SimController* controller_{nullptr};
};

}  // namespace fleetsim::ui
