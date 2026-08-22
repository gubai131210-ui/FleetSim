#pragma once

#include <QDialog>

namespace fleetsim::ui {

enum class ProjectDialogAction {
    None,
    NewProject,
    OpenProject,
    SaveProject,
};

class ProjectDialog : public QDialog {
    Q_OBJECT

public:
    explicit ProjectDialog(QWidget* parent = nullptr);

    ProjectDialogAction selectedAction() const { return selected_action_; }
    QString projectDirectory() const { return project_directory_; }

private:
    ProjectDialogAction selected_action_{ProjectDialogAction::None};
    QString project_directory_;
};

}  // namespace fleetsim::ui
