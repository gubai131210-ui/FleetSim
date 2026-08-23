#include "SimEngine.h"

#include "collision/PriorityPathCoordinator.h"
#include "control/MpcLateralTracker.h"
#include "control/StanleyTracker.h"
#include "planning/HybridAStarPlanner.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>
#include <vector>

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
    if (tracker_kind_ == "stanley" || tracker_kind_ == "pure_pursuit" ||
        tracker_kind_ == "mpc") {
        return tracker_kind_;
    }
    // auto / unknown → Pure Pursuit; mpc never implied by auto (ADR-014).
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

void SimEngine::setSpeedPlannerKind(const std::string& kind)
{
    if (kind.empty() || kind == "auto" || kind == "none") {
        speed_planner_kind_ = "none";
        return;
    }
    speed_planner_kind_ = kind;
}

std::vector<planning::PeerTrajectory> SimEngine::collectPeersFor(
    const core::VehicleId& ego_id) const
{
    std::vector<planning::PeerTrajectory> peers;
    for (std::size_t i = 0; i < fleet_.count(); ++i) {
        const vehicle::VehicleAgent& other = fleet_.agent(i);
        if (other.vehicle == nullptr || other.vehicle->id() == ego_id) {
            continue;
        }
        if (other.reference_path.empty()) {
            continue;
        }
        planning::PeerTrajectory peer;
        peer.path = other.reference_path;
        peer.nominal_speed = 0.5;
        peers.push_back(std::move(peer));
    }
    return peers;
}

void SimEngine::refreshSpeedProfileFor(vehicle::VehicleAgent& agent)
{
    if (agent.vehicle == nullptr) {
        return;
    }
    if (speed_planner_kind_ != "st_graph" || agent.reference_path.empty()) {
        agent.speed_profile = core::SpeedProfile{};
        return;
    }
    planning::StGraphSpeedPlanner planner(0.5, 0.8, 0.1);
    agent.speed_profile =
        planner.plan(agent.reference_path, collectPeersFor(agent.vehicle->id()));
}

void SimEngine::refreshSpeedProfiles()
{
    for (std::size_t i = 0; i < fleet_.count(); ++i) {
        refreshSpeedProfileFor(fleet_.agent(i));
    }
}

double SimEngine::speedFromProfile(const core::Pose& pose,
                                   const core::Path& path,
                                   const core::SpeedProfile& profile,
                                   double fallback)
{
    if (path.empty() || profile.speeds.size() != path.size()) {
        return fallback;
    }
    std::size_t best = 0;
    double best_d2 = std::numeric_limits<double>::infinity();
    const auto& w = path.waypoints();
    for (std::size_t i = 0; i < w.size(); ++i) {
        const double dx = pose.x - w[i].x;
        const double dy = pose.y - w[i].y;
        const double d2 = dx * dx + dy * dy;
        if (d2 < best_d2) {
            best_d2 = d2;
            best = i;
        }
    }
    return profile.speeds[best];
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
    refreshSpeedProfiles();
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
    const bool ok = planPathForAgent(*agent);
    if (ok) {
        refreshSpeedProfiles();
    }
    return ok;
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
    const bool ok = planPathForAgent(*agent);
    if (ok) {
        refreshSpeedProfiles();
    }
    return ok;
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
        refreshSpeedProfiles();
    }

    collision_.tick(dt, fleet_, sim_time_s_, map_);

    const bool st_enabled = (speed_planner_kind_ == "st_graph");
    const bool st_replan_now =
        st_enabled && (tick_count_ % std::max(1, st_replan_interval_ticks_) == 0);

    for (std::size_t i = 0; i < fleet_.count(); ++i) {
        vehicle::VehicleAgent& agent = fleet_.agent(i);
        if (agent.vehicle == nullptr || agent.reference_path.empty() || agent.goal_reached) {
            continue;
        }

        if (st_enabled &&
            (st_replan_now || agent.speed_profile.speeds.size() != agent.reference_path.size())) {
            refreshSpeedProfileFor(agent);
        }

        core::ControlCommand command;
        const std::string tracker_kind = resolvedTrackerKind(*agent.vehicle);
        const double max_steer =
            agent.vehicle->isBicycle() ? agent.vehicle->maxSteeringRad() : 0.6;
        const double wheelbase =
            agent.vehicle->isBicycle() ? agent.vehicle->wheelbaseM() : 0.8;
        const double profile_v = speedFromProfile(
            agent.vehicle->pose(), agent.reference_path, agent.speed_profile, 0.5);

        if (tracker_kind == "stanley") {
            control::StanleyTracker stanley(1.5, 0.1, max_steer, wheelbase, profile_v);
            command = stanley.compute(agent.vehicle->pose(), agent.reference_path, dt);
            if (st_enabled && !agent.speed_profile.speeds.empty()) {
                command.linear_velocity = profile_v;
            }
        } else if (tracker_kind == "mpc") {
            const double mpc_dt = (dt > 1e-6) ? dt : 0.05;
            control::MpcLateralTracker mpc(
                10, mpc_dt, 2.0, 2.0, 0.5, max_steer, wheelbase, 0.5);
            if (st_enabled && !agent.speed_profile.speeds.empty()) {
                mpc.setSpeedProfile(&agent.speed_profile);
            }
            command = mpc.compute(agent.vehicle->pose(), agent.reference_path, dt);
        } else if (agent.vehicle->isBicycle()) {
            command = pure_pursuit_tracker_.compute(agent.vehicle->pose(),
                                                    agent.reference_path,
                                                    dt,
                                                    agent.vehicle->wheelbaseM(),
                                                    agent.vehicle->maxSteeringRad());
            if (st_enabled && !agent.speed_profile.speeds.empty()) {
                command.linear_velocity = profile_v;
            }
        } else {
            command = pure_pursuit_tracker_.compute(
                agent.vehicle->pose(), agent.reference_path, dt);
            if (st_enabled && !agent.speed_profile.speeds.empty()) {
                command.linear_velocity = profile_v;
            }
        }
        // TimeWindow scale stacks on ST / tracker velocity (ADR-015).
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
