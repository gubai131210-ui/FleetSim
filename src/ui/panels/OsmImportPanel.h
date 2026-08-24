#pragma once

#include <QWidget>
#include <QString>

class QLabel;
class QLineEdit;
class QPushButton;

namespace fleetsim::ui {

/// OSM import dock — path, preview summary, import action (ADR-025).
class OsmImportPanel : public QWidget {
    Q_OBJECT

public:
    explicit OsmImportPanel(QWidget* parent = nullptr);

    QString osmPath() const;
    void setOsmPath(const QString& path);
    void setSummaryText(const QString& summary);

signals:
    void importRequested(const QString& osm_path);

private:
    void onBrowse();
    void onPreview();
    void onImport();

    QLineEdit* path_edit_{nullptr};
    QLabel* summary_label_{nullptr};
    QPushButton* preview_button_{nullptr};
    QPushButton* import_button_{nullptr};
};

}  // namespace fleetsim::ui
