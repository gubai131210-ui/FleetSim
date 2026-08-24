#pragma once

#include "collision/CollisionModule.h"
#include "control/PurePursuitTracker.h"
#include "domain/behavior/BtNavigator.h"
#include "domain/behavior/BtSimEngineContext.h"
#include "domain/behavior/BtTypes.h"
#include "planning/AStarPlanner.h"
#include "planning/DouglasPeuckerSmoother.h"
#include "planning/LaneRouter.h"
#include "planning/StGraphSpeedPlanner.h"
#include "domain/map/LaneGraph.h"
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

    /// speed_planner: "st_graph" | "none" (empty → none). ADR-015.
    void setSpeedPlannerKind(const std::string& kind);
    const std::string& speedPlannerKind() const { return speed_planner_kind_; }

    /// prediction: "none" | "constant_velocity" (empty → none). ADR-016.
    void setPredictionKind(const std::string& kind);
    const std::string& predictionKind() const { return prediction_kind_; }

    /// routing: "freespace" | "lane_graph" | "hybrid" (empty → freespace). ADR-018.
    void setRoutingMode(const std::string& mode);
    const std::string& routingMode() const { return routing_mode_; }
    void setLaneMap(const map::LaneMapData& lanes);
    void setLaneSnapRadiusM(double radius_m);
    double laneSnapRadiusM() const { return lane_snap_radius_m_; }
    void setFirstLastPlannerKind(const std::string& kind);
    const std::string& firstLastPlannerKind() const { return first_last_planner_kind_; }

    /// behavior: "legacy" | "bt" (default legacy — ADR-020).
    void setBehaviorMode(const std::string& mode);
    const std::string& behaviorMode() const { return behavior_mode_; }
    void setReplanHz(double hz);
    double replanHz() const { return replan_hz_; }
    void setRecoveryWaitTicks(int ticks);
    int recoveryWaitTicks() const { return recovery_wait_ticks_; }
    bool loadBehaviorTree(const std::string& json_path);
    const behavior::BtNavigator* btNavigator() const { return &bt_navigator_; }
    const behavior::BtTickResult& lastBtTickResult() const { return last_bt_result_; }

    /// Recompute SpeedProfile for all agents with paths (reads peer Paths).
    void refreshSpeedProfiles();

    bool planPath();
    bool planPathFor(const core::VehicleId& vehicle_id);
    const core::Path& referencePath() const;

    bool clearMapInflation();
    std::size_t mapOccupiedCellCount() const;

    void tick(double dt);

    int tickCount() const;
    bool goalReached() const;
    double simTimeS() const { return sim_time_s_; }

    double lastLinearVelocity() const { return last_linear_velocity_; }

private:
    bool planPathForAgent(vehicle::VehicleAgent& agent);
    bool planPathForAgentOnGrid(vehicle::VehicleAgent& agent, const map::OccupancyGrid& planning_grid);
    bool planLaneGraphPathForAgent(vehicle::VehicleAgent& agent);
    bool planHybridPathForAgent(vehicle::VehicleAgent& agent);
    core::Path planFreespaceBetween(const map::OccupancyGrid& grid,
                                    const vehicle::Vehicle& vehicle,
                                    const core::Pose& start,
                                    const core::Pose& goal,
                                    const std::string& planner_kind) const;
    std::string resolvedFirstLastPlannerKind(const vehicle::Vehicle& vehicle) const;
    static core::Path concatenatePaths(const core::Path& prefix, const core::Path& suffix);
    bool withinLaneSnap(double x, double y, const std::string& node_id) const;
    bool assignReferencePath(vehicle::VehicleAgent& agent, core::Path path);
    void replanFleetWithPriorityCoordination();
    std::vector<planning::PeerTrajectory> collectPeersFor(
        const core::VehicleId& ego_id) const;
    void refreshSpeedProfileFor(vehicle::VehicleAgent& agent);
    static double speedFromProfile(const core::Pose& pose,
                                   const core::Path& path,
                                   const core::SpeedProfile& profile,
                                   double fallback);
    void publishPoseUpdate(const vehicle::VehicleAgent& agent);
    void publishPathUpdate(const core::VehicleId& vehicle_id, const core::Path& path);
    void handleAgentGoalReached(vehicle::VehicleAgent& agent);
    void tickBehaviorTreeForAgent(vehicle::VehicleAgent& agent);
    vehicle::VehicleAgent* selectedAgent();
    const vehicle::VehicleAgent* selectedAgent() const;

    core::SimClock clock_;
    core::EventBus event_bus_;
    scheduling::SchedulingModule scheduling_;
    collision::CollisionModule collision_;

    map::OccupancyGrid map_;
    map::LaneGraph lane_graph_;
    vehicle::FleetManager fleet_;

    planning::AStarPlanner astar_planner_;
    planning::DouglasPeuckerSmoother smoother_;
    control::PurePursuitTracker pure_pursuit_tracker_;

    std::string planner_kind_{"auto"};
    std::string tracker_kind_{"auto"};
    std::string coordination_kind_{"priority"};
    std::string speed_planner_kind_{"none"};
    std::string prediction_kind_{"none"};
    std::string routing_mode_{"freespace"};
    double lane_snap_radius_m_{1.0};
    std::string first_last_planner_kind_;
    double prediction_horizon_s_{3.0};
    double prediction_sample_dt_s_{0.1};
    int st_replan_interval_ticks_{10};

    core::VehicleId selected_vehicle_id_;
    core::Pose manual_goal_;
    double sim_time_s_{0.0};
    double last_linear_velocity_{0.0};
    int tick_count_{0};

    std::string behavior_mode_{"legacy"};
    double replan_hz_{1.0};
    int recovery_wait_ticks_{20};
    behavior::BtNavigator bt_navigator_;
    std::unique_ptr<behavior::BtSimEngineContext> bt_context_;
    behavior::BtTickResult last_bt_result_;
};

}  // namespace fleetsim::domain
