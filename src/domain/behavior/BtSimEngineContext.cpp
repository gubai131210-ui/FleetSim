#include "BtSimEngineContext.h"

#include "domain/SimEngine.h"
#include "vehicle/FleetManager.h"

#include <cmath>

namespace fleetsim::domain::behavior {

namespace {

bool poseChanged(const core::Pose& a, const core::Pose& b)
{
    constexpr double kEps = 1e-6;
    return std::abs(a.x - b.x) > kEps || std::abs(a.y - b.y) > kEps
           || std::abs(a.theta - b.theta) > kEps;
}

}  // namespace

BtSimEngineContext::BtSimEngineContext(SimEngine& engine, core::VehicleId agent_id)
    : engine_(engine)
    , agent_id_(std::move(agent_id))
{
}

const vehicle::VehicleAgent* BtSimEngineContext::agent() const
{
    return engine_.fleet().findAgent(agent_id_);
}

vehicle::VehicleAgent* BtSimEngineContext::agent()
{
    return engine_.fleet().findAgent(agent_id_);
}

bool BtSimEngineContext::planPathForAgent()
{
    vehicle::VehicleAgent* mutable_agent = agent();
    if (mutable_agent == nullptr || mutable_agent->vehicle == nullptr) {
        return false;
    }
    return engine_.planPathFor(agent_id_);
}

bool BtSimEngineContext::hasValidPath() const
{
    const vehicle::VehicleAgent* current = agent();
    return current != nullptr && !current->reference_path.empty();
}

bool BtSimEngineContext::isGoalReached() const
{
    const vehicle::VehicleAgent* current = agent();
    return current != nullptr && current->goal_reached;
}

bool BtSimEngineContext::isGoalUpdated()
{
    const vehicle::VehicleAgent* current = agent();
    if (current == nullptr) {
        return false;
    }
    if (!goal_initialized_) {
        last_goal_ = current->goal;
        goal_initialized_ = true;
        return false;
    }
    return poseChanged(current->goal, last_goal_);
}

void BtSimEngineContext::acknowledgeGoal()
{
    const vehicle::VehicleAgent* current = agent();
    if (current == nullptr) {
        return;
    }
    last_goal_ = current->goal;
    goal_initialized_ = true;
}

bool BtSimEngineContext::hasReservationConflict() const
{
    return engine_.collision().conflictCount() > 0;
}

void BtSimEngineContext::requestReplan()
{
    vehicle::VehicleAgent* mutable_agent = agent();
    if (mutable_agent != nullptr) {
        mutable_agent->needs_replan = true;
    }
}

bool BtSimEngineContext::needsReplan() const
{
    const vehicle::VehicleAgent* current = agent();
    return current != nullptr && current->needs_replan;
}

void BtSimEngineContext::clearReplanRequest()
{
    vehicle::VehicleAgent* mutable_agent = agent();
    if (mutable_agent != nullptr) {
        mutable_agent->needs_replan = false;
    }
}

double BtSimEngineContext::simDt() const
{
    return engine_.clock().fixedDt();
}

}  // namespace fleetsim::domain::behavior
