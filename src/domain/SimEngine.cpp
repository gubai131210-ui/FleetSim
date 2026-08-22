#include "SimEngine.h"

#include "collision/PriorityPathCoordinator.h"
#include "control/StanleyTracker.h"
#include "planning/HybridAStarPlanner.h"

#include <nlohmann/json.hpp>

#include <cmath>

namespace fleetsim::domain {

SimEngine::SimEngine() = default;

core::SimClock& SimEngine::clock()
{
    return clock_;
}

const core::SimClock& SimEngine::clock() const
{
    return clock_;
}

core::EventBus& SimEngine::eventBus()
{
    return event_bus_;
}

const core::EventBus& SimEngine::eventBus() const
{
    return event_bus_;
}

scheduling::SchedulingModule& SimEngine::scheduling()
{
    return scheduling_;
}

const scheduling::SchedulingModule& SimEngine::scheduling() const
{
    return scheduling_;
}

collision::CollisionModule& SimEngine::collision()
{
    return collision_;
}

const collision::CollisionModule& SimEngine::collision() const
{
    return collision_;
}

vehicle::FleetManager& SimEngine::fleet()
{
    return fleet_;
}

const vehicle::FleetManager& SimEngine::fleet() const
{
    return fleet_;
}

void SimEngine::setMap(map::OccupancyGrid map)
{
    map_ = std::move(map);
}

const map::OccupancyGrid& SimEngine::map() const
{
    return map_;
}

void SimEngine::clearFleet()
{
    fleet_.clear();
    selected_vehicle_id_.clear();
}

void SimEngine::addVehicle(std::unique_ptr<vehicle::Vehicle> vehicle)
{
    if (vehicle == nullptr) {
        return;
    }
    if (selected_vehicle_id_.empty()) {
        selected_vehicle_id_ = vehicle->id();
    }
    fleet_.addVehicle(std::move(vehicle));
}

void SimEngine::setVehicle(std::unique_ptr<vehicle::Vehicle> vehicle)
{
    clearFleet();
    addVehicle(std::move(vehicle));
}

const vehicle::Vehicle* SimEngine::vehicle() const
{
    return fleet_.primaryVehicle();
}

void SimEngine::setSelectedVehicle(const core::VehicleId& vehicle_id)
{
    selected_vehicle_id_ = vehicle_id;
}

const core::VehicleId& SimEngine::selectedVehicleId() const
{
    return selected_vehicle_id_;
}

void SimEngine::loadTasks(const std::vector<core::Task>& tasks)
{
    scheduling_.loadTasks(tasks);
}

void SimEngine::addTask(const core::Task& task)
{
    scheduling_.tasks().addTask(task);
}

void SimEngine::setGoal(const core::Pose& goal)
{
    manual_goal_ = goal;
    vehicle::VehicleAgent* agent = selectedAgent();
    if (agent == nullptr) {
        return;
    }
    agent->goal = goal;
    agent->goal_reached = false;
    agent->needs_replan = true;
    agent->phase = vehicle::AgentPhase::Idle;
    agent->task_id.clear();
}

void SimEngine::setPlannerKind(const std::string& kind)
{
    if (kind.empty()) {
        planner_kind_ = "auto";
        return;
    }
    planner_kind_ = kind;
}

void SimEngine::setTrackerKind(const std::string& kind)
{
    if (kind.empty()) {
        tracker_kind_ = "auto";
        return;
    }
    tracker_kind_ = kind;
}

std::string SimEngine::resolvedPlannerKind(const vehicle::Vehicle& vehicle) const
{
    if (planner_kind_ == "astar" || planner_kind_ == "hybrid_astar") {
        return planner_kind_;
    }
    // auto / unknown → bicycle defaults to hybrid_astar (ADR-011).
    if (vehicle.isBicycle()) {
        return "hybrid_astar";
    }
    return "astar";
}

std::string SimEngine::resolvedTrackerKind(const vehicle::Vehicle& /*vehicle*/) const
{
    if (tracker_kind_ == "stanley" || tracker_kind_ == "pure_pursuit") {
        return tracker_kind_;
    }
    // auto → Pure Pursuit (DiffDrive regression + bicycle_demo default).
    return "pure_pursuit";
}

void SimEngine::setCoordinationKind(const std::string& kind)
{
    if (kind.empty() || kind == "auto") {
        coordination_kind_ = "priority";
        return;
    }
    coordination_kind_ = kind;
}

bool SimEngine::usesPriorityCoordination() const
{
    return coordination_kind_ != "none";
}

const core::Pose& SimEngine::goal() const
{
    const vehicle::VehicleAgent* agent = selectedAgent();
    if (agent != nullptr) {
        return agent->goal;
    }
    return manual_goal_;
}

vehicle::VehicleAgent* SimEngine::selectedAgent()
{
    return const_cast<vehicle::VehicleAgent*>(
        static_cast<const SimEngine*>(this)->selectedAgent());
}

const vehicle::VehicleAgent* SimEngine::selectedAgent() const
{
    if (!selected_vehicle_id_.empty()) {
        return fleet_.findAgent(selected_vehicle_id_);
    }
    return fleet_.primaryAgent();
}

bool SimEngine::planPathForAgent(vehicle::VehicleAgent& agent)
{
    return planPathForAgentOnGrid(agent, map_);
}

bool SimEngine::planPathForAgentOnGrid(vehicle::VehicleAgent& agent,
                                       const map::OccupancyGrid& planning_grid)
{
    if (agent.vehicle == nullptr || planning_grid.rows() == 0) {
        return false;
    }

    const std::string kind = resolvedPlannerKind(*agent.vehicle);
    core::Path raw_path;
    if (kind == "hybrid_astar") {
        const double wheelbase = agent.vehicle->isBicycle()
            ? agent.vehicle->wheelbaseM()
            : 0.8;
        const double max_steer = agent.vehicle->isBicycle()
            ? agent.vehicle->maxSteeringRad()
            : 0.6;
        planning::HybridAStarPlanner hybrid(wheelbase, max_steer);
        raw_path = hybrid.plan(planning_grid, agent.vehicle->pose(), agent.goal);
        agent.reference_path = raw_path;
    } else {
        raw_path = astar_planner_.plan(planning_grid, agent.vehicle->pose(), agent.goal);
        if (raw_path.empty()) {
            agent.reference_path.clear();
            publishPathUpdate(agent.vehicle->id(), agent.reference_path);
            return false;
        }
        agent.reference_path = smoother_.smooth(raw_path);
    }

    if (agent.reference_path.empty()) {
        publishPathUpdate(agent.vehicle->id(), agent.reference_path);
        return false;
    }

    agent.goal_reached = false;
    publishPathUpdate(agent.vehicle->id(), agent.reference_path);
    collision_.reservePath(
        agent.vehicle->id(), agent.reference_path, sim_time_s_, agent.task_priority, map_);
    return !agent.reference_path.empty();
}

void SimEngine::replanFleetWithPriorityCoordination()
{
    // ADR-013: clear then plan/reserve in priority order; paint higher paths as obstacles.
    collision_.clearReservations();
    map::OccupancyGrid working = map_;
    const auto order = collision::PriorityPathCoordinator::orderedAgentIndices(fleet_);

    for (std::size_t idx : order) {
        vehicle::VehicleAgent& agent = fleet_.agent(idx);
        if (agent.vehicle == nullptr) {
            continue;
        }

        if (agent.needs_replan) {
            agent.needs_replan = false;
            planPathForAgentOnGrid(agent, working);
        } else if (!agent.reference_path.empty()) {
            // Keep committed path; re-register reservation under cleared table.
            collision_.reservePath(
                agent.vehicle->id(), agent.reference_path, sim_time_s_, agent.task_priority, map_);
        }

        if (!agent.reference_path.empty()) {
            collision::PriorityPathCoordinator::paintPathOccupied(working, agent.reference_path, 1);
        }
    }
}

bool SimEngine::planPath()
{
    vehicle::VehicleAgent* agent = selectedAgent();
    if (agent == nullptr) {
        return false;
    }
    agent->needs_replan = true;
    if (usesPriorityCoordination() && fleet_.count() > 1) {
        replanFleetWithPriorityCoordination();
        return !agent->reference_path.empty();
    }
    agent->needs_replan = false;
    return planPathForAgent(*agent);
}

bool SimEngine::planPathFor(const core::VehicleId& vehicle_id)
{
    vehicle::VehicleAgent* agent = fleet_.findAgent(vehicle_id);
    if (agent == nullptr) {
        return false;
    }
    agent->needs_replan = true;
    if (usesPriorityCoordination() && fleet_.count() > 1) {
        replanFleetWithPriorityCoordination();
        return !agent->reference_path.empty();
    }
    agent->needs_replan = false;
    return planPathForAgent(*agent);
}

const core::Path& SimEngine::referencePath() const
{
    const vehicle::VehicleAgent* agent = selectedAgent();
    static const core::Path kEmptyPath;
    if (agent == nullptr) {
        return kEmptyPath;
    }
    return agent->reference_path;
}

void SimEngine::publishPoseUpdate(const vehicle::VehicleAgent& agent)
{
    if (agent.vehicle == nullptr) {
        return;
    }

    nlohmann::json payload;
    payload["id"] = agent.vehicle->id();
    payload["x"] = agent.vehicle->pose().x;
    payload["y"] = agent.vehicle->pose().y;
    payload["theta"] = agent.vehicle->pose().theta;
    payload["linear_velocity"] = agent.linear_velocity;
    payload["task_id"] = agent.task_id;
    event_bus_.publish("sim/pose_updated", payload.dump());
}

void SimEngine::publishPathUpdate(const core::VehicleId& vehicle_id, const core::Path& path)
{
    nlohmann::json payload;
    payload["id"] = vehicle_id;
    nlohmann::json waypoints = nlohmann::json::array();
    for (const core::Waypoint& waypoint : path.waypoints()) {
        waypoints.push_back({{"x", waypoint.x}, {"y", waypoint.y}});
    }
    payload["waypoints"] = waypoints;
    event_bus_.publish("sim/path_updated", payload.dump());
}

void SimEngine::handleAgentGoalReached(vehicle::VehicleAgent& agent)
{
    if (agent.phase == vehicle::AgentPhase::ToPickup) {
        agent.phase = vehicle::AgentPhase::ToDropoff;
        agent.goal = agent.dropoff_pose;
        agent.goal_reached = false;
        agent.needs_replan = true;
        event_bus_.publish("sim/task_pickup_reached", agent.task_id);
        return;
    }

    if (agent.phase == vehicle::AgentPhase::ToDropoff) {
        event_bus_.publish("sim/task_completed", agent.task_id);
        scheduling_.tasks().markDone(agent.task_id);
        fleet_.completeActiveTask(agent.vehicle->id());
        return;
    }

    if (agent.phase == vehicle::AgentPhase::Idle) {
        event_bus_.publish("sim/goal_reached", agent.vehicle->id());
    }
}

void SimEngine::tick(double dt)
{
    scheduling_.tick(dt, fleet_);

    bool any_replan = false;
    for (std::size_t i = 0; i < fleet_.count(); ++i) {
        if (fleet_.agent(i).needs_replan) {
            any_replan = true;
            break;
        }
    }

    if (any_replan && usesPriorityCoordination()) {
        replanFleetWithPriorityCoordination();
    } else if (any_replan) {
        for (std::size_t i = 0; i < fleet_.count(); ++i) {
            vehicle::VehicleAgent& agent = fleet_.agent(i);
            if (agent.needs_replan) {
                agent.needs_replan = false;
                planPathForAgent(agent);
            }
        }
    }

    collision_.tick(dt, fleet_, sim_time_s_, map_);

    for (std::size_t i = 0; i < fleet_.count(); ++i) {
        vehicle::VehicleAgent& agent = fleet_.agent(i);
        if (agent.vehicle == nullptr || agent.reference_path.empty() || agent.goal_reached) {
            continue;
        }

        core::ControlCommand command;
        const std::string tracker_kind = resolvedTrackerKind(*agent.vehicle);
        if (tracker_kind == "stanley") {
            // Rebuild with vehicle geometry so δ limits / wheelbase match the agent.
            control::StanleyTracker stanley(
                1.5,
                0.1,
                agent.vehicle->isBicycle() ? agent.vehicle->maxSteeringRad() : 0.6,
                agent.vehicle->isBicycle() ? agent.vehicle->wheelbaseM() : 0.8,
                0.5);
            command = stanley.compute(agent.vehicle->pose(), agent.reference_path, dt);
            if (!agent.vehicle->isBicycle()) {
                // DiffDrive uses ω; Stanley already fills angular_velocity from δ.
            }
        } else if (agent.vehicle->isBicycle()) {
            command = pure_pursuit_tracker_.compute(agent.vehicle->pose(),
                                                    agent.reference_path,
                                                    dt,
                                                    agent.vehicle->wheelbaseM(),
                                                    agent.vehicle->maxSteeringRad());
        } else {
            command = pure_pursuit_tracker_.compute(
                agent.vehicle->pose(), agent.reference_path, dt);
        }
        command.linear_velocity *= agent.speed_scale;
        agent.linear_velocity = command.linear_velocity;
        last_linear_velocity_ = command.linear_velocity;
        agent.vehicle->integrate(command, dt);
        publishPoseUpdate(agent);

        const core::Waypoint goal_point = agent.reference_path.waypoints().back();
        const double dx = goal_point.x - agent.vehicle->pose().x;
        const double dy = goal_point.y - agent.vehicle->pose().y;
        const double distance = std::sqrt(dx * dx + dy * dy);
        if (distance <= 0.2) {
            agent.goal_reached = true;
            handleAgentGoalReached(agent);
        }
    }

    sim_time_s_ += dt;
    ++tick_count_;
    event_bus_.publish("sim/tick", std::to_string(tick_count_));
    event_bus_.publish("sim/fleet_tick", std::to_string(tick_count_));
}

int SimEngine::tickCount() const
{
    return tick_count_;
}

bool SimEngine::goalReached() const
{
    const vehicle::VehicleAgent* agent = selectedAgent();
    return agent != nullptr && agent->goal_reached;
}

}  // namespace fleetsim::domain
