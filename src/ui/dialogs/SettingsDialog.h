#pragma once

#include <QDialog>
#include <QString>

class QComboBox;
class QDoubleSpinBox;

namespace fleetsim::ui {

struct SimulationSettings {
    QString vehicle_model{"diff_drive"};  // "diff_drive" | "bicycle"
    double wheelbase_m{0.9};
    double max_steering_rad{0.6};
    QString assigner{"greedy"};  // "greedy" | "hungarian"
};

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);

    SimulationSettings settings() const;
    void setSettings(const SimulationSettings& settings);

private:
    void rebuildModelDependentEnabled();

    QComboBox* model_combo_{nullptr};
    QDoubleSpinBox* wheelbase_spin_{nullptr};
    QDoubleSpinBox* max_steering_spin_{nullptr};
    QComboBox* assigner_combo_{nullptr};
};

}  // namespace fleetsim::ui
