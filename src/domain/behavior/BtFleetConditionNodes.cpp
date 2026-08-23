#include "BtFleetConditionNodes.h"

#include "BtTypes.h"

namespace fleetsim::domain::behavior {

namespace {

IBtSimContext* requireContext(BtBlackboard& blackboard)
{
    return blackboard.simContext();
}

}  // namespace

BtIsGoalUpdatedNode::BtIsGoalUpdatedNode(std::string name) : name_(std::move(name)) {}

NodeStatus BtIsGoalUpdatedNode::tick(BtBlackboard& blackboard)
{
    blackboard.setString(BbKey::kActiveNodeName, name_);
    IBtSimContext* context = requireContext(blackboard);
    if (context == nullptr) {
        return NodeStatus::Failure;
    }
    if (context->isGoalUpdated()) {
        blackboard.setBool(BbKey::kGoalUpdated, true);
        context->acknowledgeGoal();
        return NodeStatus::Success;
    }
    blackboard.setBool(BbKey::kGoalUpdated, false);
    return NodeStatus::Failure;
}

BtIsPathValidNode::BtIsPathValidNode(std::string name) : name_(std::move(name)) {}

NodeStatus BtIsPathValidNode::tick(BtBlackboard& blackboard)
{
    blackboard.setString(BbKey::kActiveNodeName, name_);
    IBtSimContext* context = requireContext(blackboard);
    if (context == nullptr) {
        blackboard.setBool(BbKey::kPathValid, false);
        return NodeStatus::Failure;
    }
    const bool valid = context->hasValidPath();
    blackboard.setBool(BbKey::kPathValid, valid);
    return valid ? NodeStatus::Success : NodeStatus::Failure;
}

BtYieldIfBlockedNode::BtYieldIfBlockedNode(std::string name) : name_(std::move(name)) {}

NodeStatus BtYieldIfBlockedNode::tick(BtBlackboard& blackboard)
{
    blackboard.setString(BbKey::kActiveNodeName, name_);
    IBtSimContext* context = requireContext(blackboard);
    if (context == nullptr) {
        return NodeStatus::Failure;
    }
    if (context->hasReservationConflict()) {
        context->requestReplan();
        blackboard.setBool(BbKey::kReplanRequested, true);
        return NodeStatus::Running;
    }
    return NodeStatus::Success;
}

}  // namespace fleetsim::domain::behavior
