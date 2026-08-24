#include "OsmImportPanel.h"

#include "domain/map/OsmLaneletImporter.h"

#include <QFormLayout>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace fleetsim::ui {

OsmImportPanel::OsmImportPanel(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(tr("Import OSM / Lanelet teaching subset into lane graph."), this));

    auto* form = new QFormLayout();
    path_edit_ = new QLineEdit(this);
    path_edit_->setPlaceholderText(tr("assets/maps/teaching_lanelet_subset.osm"));
    auto* browse = new QPushButton(tr("Browse…"), this);
    auto* path_row = new QWidget(this);
    auto* path_layout = new QHBoxLayout(path_row);
    path_layout->setContentsMargins(0, 0, 0, 0);
    path_layout->addWidget(path_edit_, 1);
    path_layout->addWidget(browse);
    form->addRow(tr("OSM file"), path_row);

    summary_label_ = new QLabel(tr("Preview: —"), this);
    summary_label_->setWordWrap(true);
    form->addRow(tr("Summary"), summary_label_);
    layout->addLayout(form);

    auto* buttons = new QHBoxLayout();
    preview_button_ = new QPushButton(tr("Preview"), this);
    import_button_ = new QPushButton(tr("Import to project"), this);
    buttons->addWidget(preview_button_);
    buttons->addWidget(import_button_);
    layout->addLayout(buttons);
    layout->addStretch();

    connect(browse, &QPushButton::clicked, this, &OsmImportPanel::onBrowse);
    connect(preview_button_, &QPushButton::clicked, this, &OsmImportPanel::onPreview);
    connect(import_button_, &QPushButton::clicked, this, &OsmImportPanel::onImport);
}

QString OsmImportPanel::osmPath() const
{
    return path_edit_->text().trimmed();
}

void OsmImportPanel::setOsmPath(const QString& path)
{
    path_edit_->setText(path);
}

void OsmImportPanel::setSummaryText(const QString& summary)
{
    summary_label_->setText(summary);
}

void OsmImportPanel::onBrowse()
{
    const QString path = QFileDialog::getOpenFileName(
        this, tr("Select OSM file"), QString(), tr("OSM (*.osm);;All (*.*)"));
    if (!path.isEmpty()) {
        path_edit_->setText(path);
    }
}

void OsmImportPanel::onPreview()
{
    if (osmPath().isEmpty()) {
        setSummaryText(tr("Preview failed: path empty"));
        return;
    }

    domain::map::OsmImportError error;
    const auto lanes =
        domain::map::OsmLaneletImporter::importFromFile(osmPath().toStdString(), &error);
    if (!lanes.has_value()) {
        setSummaryText(tr("Preview failed: %1")
                           .arg(QString::fromStdString(error.message)));
        return;
    }

    setSummaryText(tr("Preview OK — %1 nodes, %2 edges")
                       .arg(static_cast<qint64>(lanes->nodes.size()))
                       .arg(static_cast<qint64>(lanes->edges.size())));
}

void OsmImportPanel::onImport()
{
    if (!osmPath().isEmpty()) {
        emit importRequested(osmPath());
    }
}

}  // namespace fleetsim::ui
