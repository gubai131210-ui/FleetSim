#pragma once

#include "collision/CollisionModule.h"
#include "control/PurePursuitTracker.h"
#include "planning/AStarPlanner.h"
#include "planning/DouglasPeuckerSmoother.h"
#include "scheduling/SchedulingModule.h"
#include "vehicle/FleetManager.h"

#include "core/EventBus.h"
#include "core/SimClock.h"
#include "core/types/Path.h"
#include "core/types/Pose.h"
#include "core/types/Task.h"
#include "core/types/VehicleId.h"
#include "domain/map/OccupancyGrid.h"

#include <memory>
#include <string>

namespace fleetsim::domain {

class SimEngine {
public:
    SimEngine();

    core::SimClock& clock();
    const core::SimClock& clock() const;

    core::EventBus& eventBus();
    const core::EventBus& eventBus() const;

    scheduling::SchedulingModule& scheduling();
    const scheduling::SchedulingModule& scheduling() const;

    collision::CollisionModule& collision();
    const collision::CollisionModule& collision() const;

    vehicle::FleetManager& fleet();
    const vehicle::FleetManager& fleet() const;

    void setMap(map::OccupancyGrid map);
    const map::OccupancyGrid& map() const;

    void clearFleet();
    void addVehicle(std::unique_ptr<vehicle::Vehicle> vehicle);
    void setVehicle(std::unique_ptr<vehicle::Vehicle> vehicle);

    const vehicle::Vehicle* vehicle() const;

    void setSelectedVehicle(const core::VehicleId& vehicle_id);
    const core::VehicleId& selectedVehicleId() const;

    void loadTasks(const std::vector<core::Task>& tasks);
    void addTask(const core::Task& task);

    void setGoal(const core::Pose& goal);
    const core::Pose& goal() const;

    /// planner: "astar" | "hybrid_astar" | "auto" (empty treated as auto).
    void setPlannerKind(const std::string& kind);
    const std::string& plannerKind() const { return planner_kind_; }

    /// tracker: "pure_pursuit" | "stanley" | "mpc" | "auto" (mpc never from auto).
    void setTrackerKind(const std::string& kind);
    const std::string& trackerKind() const { return tracker_kind_; }

    /// Resolved planner for a vehicle given current kind + model defaults (ADR-011).
    std::string resolvedPlannerKind(const vehicle::Vehicle& vehicle) const;

    /// Resolved tracker (ADR-012/014). auto → pure_pursuit; mpc must be explicit.
    std::string resolvedTrackerKind(const vehicle::Vehicle& vehicle) const;

    /// coordination: "priority" | "none" (empty → priority).
    void setCoordinationKind(const std::string& kind);
    const std::string& coordinationKind() const { return coordination_kind_; }
    bool usesPriorityCoordination() const;

    bool planPath();
    bool planPathFor(const core::VehicleId& vehicle_id);
    const core::Path& referencePath() const;

    void tick(double dt);

    int tickCount() const;
    bool goalReached() const;
    double simTimeS() const { return sim_time_s_; }

    double lastLinearVelocity() const { return last_linear_velocity_; }

private:
    bool planPathForAgent(vehicle::VehicleAgent& agent);
    bool planPathForAgentOnGrid(vehicle::VehicleAgent& agent, const map::OccupancyGrid& planning_grid);
    void replanFleetWithPriorityCoordination();
    void publishPoseUpdate(const vehicle::VehicleAgent& agent);
    void publishPathUpdate(const core::VehicleId& vehicle_id, const core::Path& path);
    void handleAgentGoalReached(vehicle::VehicleAgent& agent);
    vehicle::VehicleAgent* selectedAgent();
    const vehicle::VehicleAgent* selectedAgent() const;

    core::SimClock clock_;
    core::EventBus event_bus_;
    scheduling::SchedulingModule scheduling_;
    collision::CollisionModule collision_;

    map::OccupancyGrid map_;
    vehicle::FleetManager fleet_;

    planning::AStarPlanner astar_planner_;
    planning::DouglasPeuckerSmoother smoother_;
    control::PurePursuitTracker pure_pursuit_tracker_;

    std::string planner_kind_{"auto"};
    std::string tracker_kind_{"auto"};
    std::string coordination_kind_{"priority"};

    core::VehicleId selected_vehicle_id_;
    core::Pose manual_goal_;
    double sim_time_s_{0.0};
    double last_linear_velocity_{0.0};
    int tick_count_{0};
};

}  // namespace fleetsim::domain
