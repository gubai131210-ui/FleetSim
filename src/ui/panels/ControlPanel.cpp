#include "ControlPanel.h"

#include "app/SimController.h"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace fleetsim::ui {

ControlPanel::ControlPanel(app::SimController* controller, QWidget* parent)
    : QWidget(parent)
    , controller_(controller)
{
    setupUi();
}

void ControlPanel::setupUi()
{
    auto* root_layout = new QVBoxLayout(this);
    root_layout->setContentsMargins(8, 8, 8, 8);

    auto* group = new QGroupBox(tr("Simulation Control"), this);
    auto* group_layout = new QVBoxLayout(group);

    auto* hint = new QLabel(tr("Transport controls for the simulation clock."), group);
    hint->setWordWrap(true);
    group_layout->addWidget(hint);

    auto* button_row = new QHBoxLayout();
    auto* play_btn = new QPushButton(tr("Play"), group);
    auto* pause_btn = new QPushButton(tr("Pause"), group);
    auto* step_btn = new QPushButton(tr("Step"), group);

    button_row->addWidget(play_btn);
    button_row->addWidget(pause_btn);
    button_row->addWidget(step_btn);
    group_layout->addLayout(button_row);

    root_layout->addWidget(group);
    root_layout->addStretch();

    connect(play_btn, &QPushButton::clicked, this, [this]() {
        if (controller_ != nullptr) {
            controller_->start();
            emit statusMessage(tr("Simulation running"));
        }
    });

    connect(pause_btn, &QPushButton::clicked, this, [this]() {
        if (controller_ != nullptr) {
            controller_->pause();
            emit statusMessage(tr("Simulation paused"));
        }
    });

    connect(step_btn, &QPushButton::clicked, this, [this]() {
        if (controller_ != nullptr) {
            controller_->stepOnce();
            emit statusMessage(
                tr("Tick count: %1").arg(controller_->engine().tickCount()));
        }
    });
}

}  // namespace fleetsim::ui
