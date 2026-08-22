#include "FleetManager.h"

namespace fleetsim::domain::vehicle {

void FleetManager::clear()
{
    agents_.clear();
}

void FleetManager::addVehicle(std::unique_ptr<Vehicle> vehicle)
{
    VehicleAgent agent;
    agent.vehicle = std::move(vehicle);
    agents_.push_back(std::move(agent));
}

VehicleAgent& FleetManager::agent(std::size_t index)
{
    return agents_.at(index);
}

const VehicleAgent& FleetManager::agent(std::size_t index) const
{
    return agents_.at(index);
}

VehicleAgent* FleetManager::findAgent(const core::VehicleId& id)
{
    for (VehicleAgent& agent : agents_) {
        if (agent.vehicle->id() == id) {
            return &agent;
        }
    }
    return nullptr;
}

const VehicleAgent* FleetManager::findAgent(const core::VehicleId& id) const
{
    for (const VehicleAgent& agent : agents_) {
        if (agent.vehicle->id() == id) {
            return &agent;
        }
    }
    return nullptr;
}

const Vehicle* FleetManager::primaryVehicle() const
{
    if (agents_.empty()) {
        return nullptr;
    }
    return agents_.front().vehicle.get();
}

VehicleAgent* FleetManager::primaryAgent()
{
    if (agents_.empty()) {
        return nullptr;
    }
    return &agents_.front();
}

const VehicleAgent* FleetManager::primaryAgent() const
{
    if (agents_.empty()) {
        return nullptr;
    }
    return &agents_.front();
}

void FleetManager::startTask(const core::VehicleId& vehicle_id, const core::Task& task)
{
    VehicleAgent* agent = findAgent(vehicle_id);
    if (agent == nullptr) {
        return;
    }

    agent->task_id = task.id;
    agent->task_priority = task.priority;
    agent->dropoff_pose = task.dropoff;
    agent->goal = task.pickup;
    agent->phase = AgentPhase::ToPickup;
    agent->goal_reached = false;
    agent->needs_replan = true;
    agent->reference_path.clear();
}

void FleetManager::completeActiveTask(const core::VehicleId& vehicle_id)
{
    VehicleAgent* agent = findAgent(vehicle_id);
    if (agent == nullptr) {
        return;
    }

    agent->task_id.clear();
    agent->task_priority = 0;
    agent->phase = AgentPhase::Idle;
    agent->goal_reached = false;
    agent->needs_replan = false;
    agent->reference_path.clear();
    agent->dropoff_pose = {};
}

std::vector<core::VehicleState> FleetManager::idleVehicleStates() const
{
    std::vector<core::VehicleState> states;
    for (const VehicleAgent& agent : agents_) {
        if (agent.phase != AgentPhase::Idle) {
            continue;
        }
        core::VehicleState state;
        state.id = agent.vehicle->id();
        state.pose = agent.vehicle->pose();
        state.linear_velocity = agent.linear_velocity;
        state.idle = true;
        states.push_back(state);
    }
    return states;
}

}  // namespace fleetsim::domain::vehicle
