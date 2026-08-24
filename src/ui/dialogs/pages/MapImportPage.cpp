#include "MapImportPage.h"

#include <QComboBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace fleetsim::ui {

MapImportPage::MapImportPage(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(
        tr("HD map source for lane graph (separate from obstacle map.json)."), this));

    auto* form = new QFormLayout();
    map_source_combo_ = new QComboBox(this);
    map_source_combo_->addItem(tr("JSON lanes (map.json)"), QStringLiteral("json"));
    map_source_combo_->addItem(tr("OSM / Lanelet subset"), QStringLiteral("osm"));
    form->addRow(tr("Map source"), map_source_combo_);

    auto* path_row = new QWidget(this);
    auto* path_layout = new QHBoxLayout(path_row);
    path_layout->setContentsMargins(0, 0, 0, 0);
    osm_path_edit_ = new QLineEdit(path_row);
    osm_path_edit_->setPlaceholderText(tr("teaching_lanelet_subset.osm"));
    browse_button_ = new QPushButton(tr("Browse…"), path_row);
    path_layout->addWidget(osm_path_edit_, 1);
    path_layout->addWidget(browse_button_);
    form->addRow(tr("OSM path"), path_row);
    layout->addLayout(form);
    layout->addStretch();

    connect(browse_button_, &QPushButton::clicked, this, &MapImportPage::onBrowseOsm);
    connect(map_source_combo_, &QComboBox::currentIndexChanged, this, [this]() {
        const bool osm = map_source_combo_->currentData().toString() == QStringLiteral("osm");
        osm_path_edit_->setEnabled(osm);
        browse_button_->setEnabled(osm);
    });
    osm_path_edit_->setEnabled(false);
    browse_button_->setEnabled(false);
}

QString MapImportPage::mapSource() const
{
    return map_source_combo_->currentData().toString();
}

QString MapImportPage::osmPath() const
{
    return osm_path_edit_->text().trimmed();
}

void MapImportPage::setMapSource(const QString& source)
{
    const int index = map_source_combo_->findData(source);
    if (index >= 0) {
        map_source_combo_->setCurrentIndex(index);
    }
}

void MapImportPage::setOsmPath(const QString& path)
{
    osm_path_edit_->setText(path);
}

void MapImportPage::onBrowseOsm()
{
    const QString path = QFileDialog::getOpenFileName(
        this,
        tr("Select OSM map"),
        QString(),
        tr("OSM files (*.osm);;All files (*.*)"));
    if (!path.isEmpty()) {
        osm_path_edit_->setText(path);
    }
}

}  // namespace fleetsim::ui
