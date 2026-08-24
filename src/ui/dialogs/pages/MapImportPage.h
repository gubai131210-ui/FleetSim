#pragma once

#include <QWidget>
#include <QString>

class QComboBox;
class QLineEdit;
class QPushButton;

namespace fleetsim::ui {

/// Workbench Tab 7 — HD map import options only (ADR-022 / ADR-025).
class MapImportPage : public QWidget {
    Q_OBJECT

public:
    explicit MapImportPage(QWidget* parent = nullptr);

    QString mapSource() const;
    QString osmPath() const;

    void setMapSource(const QString& source);
    void setOsmPath(const QString& path);

private:
    void onBrowseOsm();

    QComboBox* map_source_combo_{nullptr};
    QLineEdit* osm_path_edit_{nullptr};
    QPushButton* browse_button_{nullptr};
};

}  // namespace fleetsim::ui
