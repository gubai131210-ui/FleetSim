#include "SimController.h"

#include "domain/vehicle/Vehicle.h"
#include "domain/vehicle/VehicleModelFactory.h"

#include <memory>

namespace fleetsim::app {

SimController::SimController() = default;

domain::SimEngine& SimController::engine()
{
    return engine_;
}

const domain::SimEngine& SimController::engine() const
{
    return engine_;
}

void SimController::applyScenarioToEngine()
{
    engine_.setMap(scenario_.map);
    engine_.clock().setFixedDt(scenario_.simulation.dt_s);
    engine_.clearFleet();

    for (const auto& vehicle_config : scenario_.vehicles) {
        auto model = domain::vehicle::createVehicleModel(
            vehicle_config.model,
            0.5,
            1.0,
            vehicle_config.wheelbase_m,
            vehicle_config.max_steering_rad);
        auto vehicle = std::make_unique<domain::vehicle::Vehicle>(
            vehicle_config.id,
            vehicle_config.length_m,
            vehicle_config.initial_pose,
            std::move(model));
        vehicle->setModelKind(vehicle_config.model);
        if (vehicle_config.model == "bicycle") {
            vehicle->setWheelbaseM(vehicle_config.wheelbase_m);
            vehicle->setMaxSteeringRad(vehicle_config.max_steering_rad);
        }
        engine_.addVehicle(std::move(vehicle));
    }

    if (!scenario_.vehicles.empty()) {
        engine_.setSelectedVehicle(scenario_.vehicles.front().id);
    }

    engine_.loadTasks(scenario_.tasks);
    engine_.collision().clearReservations();
}

bool SimController::loadScenario(const std::string& scenario_directory)
{
    domain::scenario::ScenarioData loaded =
        domain::scenario::ScenarioLoader::loadFromDirectory(scenario_directory);
    return loadScenarioData(std::move(loaded));
}

bool SimController::loadScenarioData(domain::scenario::ScenarioData scenario)
{
    scenario_ = std::move(scenario);
    if (scenario_.vehicles.empty()) {
        scenario_loaded_ = false;
        return false;
    }

    applyScenarioToEngine();
    scenario_loaded_ = true;
    return true;
}

const domain::scenario::ScenarioData* SimController::scenario() const
{
    return scenario_loaded_ ? &scenario_ : nullptr;
}

void SimController::setGoal(double x, double y, double theta)
{
    core::Pose goal;
    goal.x = x;
    goal.y = y;
    goal.theta = theta;
    engine_.setGoal(goal);
}

bool SimController::planPath()
{
    return engine_.planPath();
}

void SimController::addTask(const core::Task& task)
{
    scenario_.tasks.push_back(task);
    engine_.addTask(task);
}

void SimController::selectVehicle(const core::VehicleId& vehicle_id)
{
    engine_.setSelectedVehicle(vehicle_id);
}

const core::VehicleId& SimController::selectedVehicleId() const
{
    return engine_.selectedVehicleId();
}

void SimController::start()
{
    running_ = true;
    engine_.clock().resume();
}

void SimController::pause()
{
    running_ = false;
    engine_.clock().pause();
}

void SimController::stepOnce()
{
    const double dt = engine_.clock().fixedDt();
    engine_.clock().tick();
    engine_.tick(dt);
}

void SimController::tick()
{
    if (!running_) {
        return;
    }

    const double dt = engine_.clock().tick();
    if (dt > 0.0) {
        engine_.tick(dt);
    }
}

void SimController::setTimeScale(double scale)
{
    engine_.clock().setTimeScale(scale);
}

bool SimController::isRunning() const
{
    return running_;
}

}  // namespace fleetsim::app
