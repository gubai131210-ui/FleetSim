#pragma once

#include "core/types/Path.h"
#include "core/types/Pose.h"
#include "core/types/SpeedProfile.h"
#include "core/types/Task.h"
#include "core/types/VehicleState.h"
#include "vehicle/Vehicle.h"

#include <memory>
#include <vector>

namespace fleetsim::domain::vehicle {

enum class AgentPhase {
    Idle,
    ToPickup,
    ToDropoff,
};

struct VehicleAgent {
    std::unique_ptr<Vehicle> vehicle;
    core::Path reference_path;
    core::SpeedProfile speed_profile;
    core::Pose goal;
    core::TaskId task_id;
    int task_priority{0};
    AgentPhase phase{AgentPhase::Idle};
    bool goal_reached{false};
    bool needs_replan{false};
    double linear_velocity{0.0};
    double speed_scale{1.0};
    bool last_mpc_solve_ok{true};
    core::Pose dropoff_pose;
};

class FleetManager {
public:
    void clear();
    void addVehicle(std::unique_ptr<Vehicle> vehicle);

    std::size_t count() const { return agents_.size(); }
    VehicleAgent& agent(std::size_t index);
    const VehicleAgent& agent(std::size_t index) const;

    VehicleAgent* findAgent(const core::VehicleId& id);
    const VehicleAgent* findAgent(const core::VehicleId& id) const;

    const Vehicle* primaryVehicle() const;
    VehicleAgent* primaryAgent();
    const VehicleAgent* primaryAgent() const;

    void startTask(const core::VehicleId& vehicle_id, const core::Task& task);
    void completeActiveTask(const core::VehicleId& vehicle_id);

    std::vector<core::VehicleState> idleVehicleStates() const;

private:
    std::vector<VehicleAgent> agents_;
};

}  // namespace fleetsim::domain::vehicle
