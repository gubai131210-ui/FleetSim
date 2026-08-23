#pragma once

#include "AlgorithmWorkbenchDialog.h"

namespace fleetsim::ui {

using PlannerTrackerSettings = AlgorithmWorkbenchSettings;

/// @deprecated Thin wrapper — use AlgorithmWorkbenchDialog (ADR-017).
class PlannerTrackerDialog : public AlgorithmWorkbenchDialog {
public:
    explicit PlannerTrackerDialog(QWidget* parent = nullptr)
        : AlgorithmWorkbenchDialog(parent)
    {
        setWindowTitle(tr("Planner / Tracker"));
    }
};

}  // namespace fleetsim::ui
