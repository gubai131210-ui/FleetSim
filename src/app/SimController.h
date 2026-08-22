#pragma once

#include "domain/SimEngine.h"
#include "domain/scenario/ScenarioLoader.h"

#include "core/types/Task.h"

#include <string>

namespace fleetsim::app {

class SimController {
public:
    SimController();

    domain::SimEngine& engine();
    const domain::SimEngine& engine() const;

    bool loadScenario(const std::string& scenario_directory);
    bool loadScenarioData(domain::scenario::ScenarioData scenario);
    const domain::scenario::ScenarioData* scenario() const;

    void setGoal(double x, double y, double theta = 0.0);
    bool planPath();

    void addTask(const core::Task& task);
    void selectVehicle(const core::VehicleId& vehicle_id);
    const core::VehicleId& selectedVehicleId() const;

    void start();
    void pause();
    void stepOnce();
    void tick();

    void setTimeScale(double scale);

    bool isRunning() const;

private:
    void applyScenarioToEngine();

    domain::SimEngine engine_;
    domain::scenario::ScenarioData scenario_;
    bool scenario_loaded_{false};
    bool running_{false};
};

}  // namespace fleetsim::app
