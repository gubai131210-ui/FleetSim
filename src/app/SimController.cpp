#include "SimController.h"

#include "domain/vehicle/Vehicle.h"
#include "domain/vehicle/VehicleModelFactory.h"

#include <filesystem>
#include <memory>

namespace fleetsim::app {

namespace {

std::string resolveBehaviorTreePath(const domain::scenario::ScenarioData& scenario,
                                    const std::string& configured_path)
{
    if (configured_path.empty()) {
        return {};
    }

    const std::filesystem::path configured(configured_path);
    if (configured.is_absolute()) {
        return configured.string();
    }

    if (!scenario.scenario_directory.empty()) {
        const std::filesystem::path in_scenario =
            std::filesystem::path(scenario.scenario_directory) / configured;
        if (std::filesystem::exists(in_scenario)) {
            return in_scenario.string();
        }
    }

    const std::filesystem::path in_assets =
        std::filesystem::path("assets") / "behavior_trees" / configured.filename();
    if (std::filesystem::exists(in_assets)) {
        return in_assets.string();
    }

    return configured.string();
}

std::string resolveScenarioBehaviorTreePath(const domain::scenario::ScenarioData& scenario)
{
    return resolveBehaviorTreePath(scenario, scenario.simulation.behavior_tree_path);
}

}  // namespace

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

    // ADR-011: scenario planner/tracker; empty → SimEngine auto defaults.
    engine_.setPlannerKind(
        scenario_.simulation.planner.empty() ? "auto" : scenario_.simulation.planner);
    engine_.setTrackerKind(
        scenario_.simulation.tracker.empty() ? "auto" : scenario_.simulation.tracker);
    engine_.setCoordinationKind(
        scenario_.simulation.coordination.empty() ? "priority" : scenario_.simulation.coordination);
    engine_.setSpeedPlannerKind(
        scenario_.simulation.speed_planner.empty() ? "none" : scenario_.simulation.speed_planner);
    engine_.setPredictionKind(
        scenario_.simulation.prediction.empty() ? "none" : scenario_.simulation.prediction);
    engine_.setRoutingMode(
        scenario_.simulation.routing_mode.empty() ? "freespace" : scenario_.simulation.routing_mode);
    engine_.setLaneMap(scenario_.lanes);
    engine_.setLaneSnapRadiusM(scenario_.simulation.lane_snap_radius_m);
    engine_.setFirstLastPlannerKind(scenario_.simulation.first_last_planner);
    engine_.setBehaviorMode(
        scenario_.simulation.behavior_mode.empty() ? "legacy" : scenario_.simulation.behavior_mode);
    engine_.setReplanHz(scenario_.simulation.replan_hz);
    engine_.setRecoveryWaitTicks(scenario_.simulation.recovery_wait_ticks);
    engine_.setBtFormat(scenario_.simulation.bt_format);

    domain::collision::CbsLiteConfig cbs_config;
    cbs_config.max_depth = scenario_.simulation.cbs_max_depth;
    cbs_config.time_limit_ms = scenario_.simulation.cbs_time_limit_ms;
    engine_.setCbsLiteConfig(cbs_config);

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

    if (engine_.behaviorMode() == "bt") {
        const std::string default_tree_path = resolveScenarioBehaviorTreePath(scenario_);
        if (!default_tree_path.empty()) {
            engine_.loadBehaviorTree(default_tree_path, scenario_.simulation.bt_format);
        }
        for (const auto& vehicle_config : scenario_.vehicles) {
            if (vehicle_config.behavior_tree_path.empty()) {
                continue;
            }
            const std::string tree_path =
                resolveBehaviorTreePath(scenario_, vehicle_config.behavior_tree_path);
            if (!tree_path.empty()) {
                engine_.loadBehaviorTreeForAgent(
                    vehicle_config.id, tree_path, scenario_.simulation.bt_format);
            }
        }
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
