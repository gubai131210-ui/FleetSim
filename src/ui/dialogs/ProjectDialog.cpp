#include "ProjectDialog.h"

#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace fleetsim::ui {

ProjectDialog::ProjectDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Project"));
    resize(480, 160);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(tr("Scenario directory (contains map.json + scenario.json):"), this));

    auto* path_row = new QHBoxLayout();
    auto* path_edit = new QLineEdit(this);
    auto* browse_button = new QPushButton(tr("Browse..."), this);
    path_row->addWidget(path_edit);
    path_row->addWidget(browse_button);
    layout->addLayout(path_row);

    auto* button_row = new QHBoxLayout();
    auto* new_button = new QPushButton(tr("New"), this);
    auto* open_button = new QPushButton(tr("Open"), this);
    auto* save_button = new QPushButton(tr("Save"), this);
    auto* cancel_button = new QPushButton(tr("Cancel"), this);
    button_row->addWidget(new_button);
    button_row->addWidget(open_button);
    button_row->addWidget(save_button);
    button_row->addStretch();
    button_row->addWidget(cancel_button);
    layout->addLayout(button_row);

    connect(browse_button, &QPushButton::clicked, this, [this, path_edit]() {
        const QString dir = QFileDialog::getExistingDirectory(this, tr("Select scenario directory"));
        if (!dir.isEmpty()) {
            path_edit->setText(dir);
            project_directory_ = dir;
        }
    });

    auto select_action = [this, path_edit](ProjectDialogAction action) {
        project_directory_ = path_edit->text().trimmed();
        if (project_directory_.isEmpty()) {
            return;
        }
        selected_action_ = action;
        accept();
    };

    connect(new_button, &QPushButton::clicked, this, [select_action]() {
        select_action(ProjectDialogAction::NewProject);
    });
    connect(open_button, &QPushButton::clicked, this, [select_action]() {
        select_action(ProjectDialogAction::OpenProject);
    });
    connect(save_button, &QPushButton::clicked, this, [select_action]() {
        select_action(ProjectDialogAction::SaveProject);
    });
    connect(cancel_button, &QPushButton::clicked, this, &QDialog::reject);
}

}  // namespace fleetsim::ui
