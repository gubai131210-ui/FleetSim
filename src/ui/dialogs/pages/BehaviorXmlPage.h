#pragma once

#include <QWidget>
#include <QString>

class QComboBox;
class QDoubleSpinBox;
class QLineEdit;
class QPushButton;

namespace fleetsim::ui {

/// Workbench Tab 8 — BT XML / motion recovery parameters (ADR-023 / ADR-025).
class BehaviorXmlPage : public QWidget {
    Q_OBJECT

public:
    explicit BehaviorXmlPage(QWidget* parent = nullptr);

    QString btFormat() const;
    QString behaviorTreePath() const;
    double spinRad() const;
    double backupDistM() const;
    double backupSpeedMps() const;

    void setBtFormat(const QString& format);
    void setBehaviorTreePath(const QString& path);
    void setSpinRad(double rad);
    void setBackupDistM(double meters);
    void setBackupSpeedMps(double meters_per_second);

private:
    void onBrowseTree();

    QComboBox* bt_format_combo_{nullptr};
    QLineEdit* tree_path_edit_{nullptr};
    QPushButton* browse_button_{nullptr};
    QDoubleSpinBox* spin_rad_spin_{nullptr};
    QDoubleSpinBox* backup_dist_spin_{nullptr};
    QDoubleSpinBox* backup_speed_spin_{nullptr};
};

}  // namespace fleetsim::ui
