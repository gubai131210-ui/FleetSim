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

    auto* speed_group = new QGroupBox(tr("Speed"), this);
    auto* speed_layout = new QHBoxLayout(speed_group);
    auto* speed_1x = new QPushButton(tr("1x"), speed_group);
    auto* speed_2x = new QPushButton(tr("2x"), speed_group);
    auto* speed_4x = new QPushButton(tr("4x"), speed_group);
    speed_layout->addWidget(speed_1x);
    speed_layout->addWidget(speed_2x);
    speed_layout->addWidget(speed_4x);
    root_layout->addWidget(speed_group);

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

    connect(speed_1x, &QPushButton::clicked, this, [this]() {
        if (controller_ != nullptr) {
            controller_->setTimeScale(1.0);
            emit statusMessage(tr("Speed 1x"));
        }
    });
    connect(speed_2x, &QPushButton::clicked, this, [this]() {
        if (controller_ != nullptr) {
            controller_->setTimeScale(2.0);
            emit statusMessage(tr("Speed 2x"));
        }
    });
    connect(speed_4x, &QPushButton::clicked, this, [this]() {
        if (controller_ != nullptr) {
            controller_->setTimeScale(4.0);
            emit statusMessage(tr("Speed 4x"));
        }
    });
}

}  // namespace fleetsim::ui
