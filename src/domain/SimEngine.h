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
    void publishPoseUpdate(const vehicle::VehicleAgent& agent);
    void publishPathUpdate(const core::VehicleId& vehicle_id, const core::Path& path);
    void handleAgentGoalReached(vehicle::VehicleAgent& agent);
    vehicle::VehicleAgent* selectedAgent();

    core::SimClock clock_;
    core::EventBus event_bus_;
    scheduling::SchedulingModule scheduling_;
    collision::CollisionModule collision_;

    map::OccupancyGrid map_;
    vehicle::FleetManager fleet_;

    planning::AStarPlanner planner_;
    planning::DouglasPeuckerSmoother smoother_;
    control::PurePursuitTracker tracker_;

    core::VehicleId selected_vehicle_id_;
    core::Pose manual_goal_;
    double sim_time_s_{0.0};
    double last_linear_velocity_{0.0};
    int tick_count_{0};
};

}  // namespace fleetsim::domain
