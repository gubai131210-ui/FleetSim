#pragma once

#include <QWidget>
#include <QString>

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLineEdit;
class QPushButton;
class QSpinBox;

namespace fleetsim::ui {

/// Workbench Tab 6 — behavior orchestration only (ADR-021). No planner/routing/lane controls.
class BehaviorPage : public QWidget {
    Q_OBJECT

public:
    explicit BehaviorPage(QWidget* parent = nullptr);

    QString behaviorMode() const;
    QString behaviorTreePath() const;
    double replanHz() const;
    int recoveryWaitTicks() const;
    bool recoveryEnabled() const;

    void setBehaviorMode(const QString& mode);
    void setBehaviorTreePath(const QString& path);
    void setReplanHz(double hz);
    void setRecoveryWaitTicks(int ticks);
    void setRecoveryEnabled(bool enabled);

signals:
    void browseTreeFileRequested();

private:
    void onBrowseClicked();

    QComboBox* behavior_mode_combo_{nullptr};
    QLineEdit* tree_path_edit_{nullptr};
    QPushButton* browse_button_{nullptr};
    QDoubleSpinBox* replan_hz_spin_{nullptr};
    QSpinBox* recovery_ticks_spin_{nullptr};
    QCheckBox* recovery_enabled_{nullptr};
};

}  // namespace fleetsim::ui
