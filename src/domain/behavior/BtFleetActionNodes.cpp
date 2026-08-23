#include "BtFleetActionNodes.h"

#include "BtTypes.h"

#include <algorithm>
#include <cmath>

namespace fleetsim::domain::behavior {

namespace {

IBtSimContext* requireContext(BtBlackboard& blackboard)
{
    return blackboard.simContext();
}

int replanCooldownTicks(const BtBlackboard& blackboard, const IBtSimContext& context)
{
    const double hz = blackboard.getDouble(BbKey::kReplanHz).value_or(1.0);
    const double dt = context.simDt();
    if (hz <= 0.0 || dt <= 0.0) {
        return 1;
    }
    return std::max(1, static_cast<int>(std::lround(1.0 / (hz * dt))));
}

int recoveryWaitTicks(const BtBlackboard& blackboard)
{
    return std::max(0, blackboard.getInt(BbKey::kRecoveryWaitTicks).value_or(20));
}

}  // namespace

BtPlanPathNode::BtPlanPathNode(std::string name) : name_(std::move(name)) {}

NodeStatus BtPlanPathNode::tick(BtBlackboard& blackboard)
{
    blackboard.setString(BbKey::kActiveNodeName, name_);
    IBtSimContext* context = requireContext(blackboard);
    if (context == nullptr) {
        blackboard.setBool(BbKey::kPathValid, false);
        return NodeStatus::Failure;
    }

    const bool ok = context->planPathForAgent();
    blackboard.setBool(BbKey::kPathValid, context->hasValidPath());
    blackboard.setBool(BbKey::kReplanRequested, false);
    if (ok) {
        context->clearReplanRequest();
    }
    return ok ? NodeStatus::Success : NodeStatus::Failure;
}

BtFollowUntilGoalNode::BtFollowUntilGoalNode(std::string name) : name_(std::move(name)) {}

NodeStatus BtFollowUntilGoalNode::tick(BtBlackboard& blackboard)
{
    blackboard.setString(BbKey::kActiveNodeName, name_);
    IBtSimContext* context = requireContext(blackboard);
    if (context == nullptr) {
        return NodeStatus::Failure;
    }
    if (context->isGoalReached()) {
        return NodeStatus::Success;
    }
    if (!context->hasValidPath()) {
        return NodeStatus::Failure;
    }
    return NodeStatus::Running;
}

BtWaitRecoveryNode::BtWaitRecoveryNode(std::string name) : name_(std::move(name)) {}

NodeStatus BtWaitRecoveryNode::tick(BtBlackboard& blackboard)
{
    blackboard.setString(BbKey::kActiveNodeName, name_);
    if (ticks_remaining_ < 0) {
        ticks_remaining_ = recoveryWaitTicks(blackboard);
    }
    if (ticks_remaining_ <= 0) {
        ticks_remaining_ = -1;
        return NodeStatus::Success;
    }
    --ticks_remaining_;
    return NodeStatus::Running;
}

BtReplanIfTimerNode::BtReplanIfTimerNode(std::string name) : name_(std::move(name)) {}

NodeStatus BtReplanIfTimerNode::tick(BtBlackboard& blackboard)
{
    blackboard.setString(BbKey::kActiveNodeName, name_);
    IBtSimContext* context = requireContext(blackboard);
    if (context == nullptr) {
        return NodeStatus::Failure;
    }
    if (context->needsReplan()) {
        blackboard.setBool(BbKey::kReplanRequested, true);
        return NodeStatus::Success;
    }
    if (cooldown_ticks_remaining_ > 0) {
        --cooldown_ticks_remaining_;
        return NodeStatus::Failure;
    }
    context->requestReplan();
    blackboard.setBool(BbKey::kReplanRequested, true);
    cooldown_ticks_remaining_ = replanCooldownTicks(blackboard, *context);
    return NodeStatus::Success;
}

}  // namespace fleetsim::domain::behavior
