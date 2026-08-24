#include "BtControlNodes.h"

#include "BtTypes.h"

namespace fleetsim::domain::behavior {

namespace {

NodeStatus tickChildrenSequence(const std::vector<BtNodePtr>& children,
                                BtBlackboard& blackboard,
                                std::string& active_child_name)
{
    active_child_name.clear();
    if (children.empty()) {
        return NodeStatus::Success;
    }

    for (const auto& child : children) {
        active_child_name = child->name();
        const NodeStatus status = child->tick(blackboard);
        if (status != NodeStatus::Success) {
            return status;
        }
    }
    return NodeStatus::Success;
}

NodeStatus tickChildrenFallback(const std::vector<BtNodePtr>& children,
                                BtBlackboard& blackboard,
                                std::string& active_child_name)
{
    active_child_name.clear();
    if (children.empty()) {
        return NodeStatus::Failure;
    }

    for (const auto& child : children) {
        active_child_name = child->name();
        const NodeStatus status = child->tick(blackboard);
        if (status == NodeStatus::Success) {
            return NodeStatus::Success;
        }
        if (status == NodeStatus::Running) {
            return NodeStatus::Running;
        }
    }
    return NodeStatus::Failure;
}

}  // namespace

BtSequenceNode::BtSequenceNode(std::string name, std::vector<BtNodePtr> children)
    : name_(std::move(name))
    , children_(std::move(children))
{
}

NodeStatus BtSequenceNode::tick(BtBlackboard& blackboard)
{
    blackboard.setString(BbKey::kActiveNodeName, name_);
    return tickChildrenSequence(children_, blackboard, active_child_name_);
}

BtFallbackNode::BtFallbackNode(std::string name, std::vector<BtNodePtr> children)
    : name_(std::move(name))
    , children_(std::move(children))
{
}

NodeStatus BtFallbackNode::tick(BtBlackboard& blackboard)
{
    blackboard.setString(BbKey::kActiveNodeName, name_);
    return tickChildrenFallback(children_, blackboard, active_child_name_);
}

BtRecoveryNode::BtRecoveryNode(std::string name,
                               BtNodePtr primary,
                               BtNodePtr recovery,
                               int retries)
    : name_(std::move(name))
    , primary_(std::move(primary))
    , recovery_(std::move(recovery))
    , max_retries_(retries)
    , retries_remaining_(retries)
{
}

NodeStatus BtRecoveryNode::tick(BtBlackboard& blackboard)
{
    blackboard.setString(BbKey::kActiveNodeName, name_);
    active_child_name_.clear();

    if (!primary_) {
        return NodeStatus::Failure;
    }

    // Nav2 RecoveryNode: primary first; on FAILURE run recovery then retry primary.
    while (true) {
        active_child_name_ = primary_->name();
        const NodeStatus primary_status = primary_->tick(blackboard);
        if (primary_status == NodeStatus::Success) {
            return NodeStatus::Success;
        }
        if (primary_status == NodeStatus::Running) {
            return NodeStatus::Running;
        }

        if (retries_remaining_ <= 0) {
            return NodeStatus::Failure;
        }

        if (!recovery_) {
            return NodeStatus::Failure;
        }

        active_child_name_ = recovery_->name();
        const NodeStatus recovery_status = recovery_->tick(blackboard);
        if (recovery_status == NodeStatus::Running) {
            return NodeStatus::Running;
        }
        if (recovery_status == NodeStatus::Failure) {
            return NodeStatus::Failure;
        }

        --retries_remaining_;
        const int recovery_count =
            blackboard.getInt(BbKey::kRecoveryCount).value_or(0) + 1;
        blackboard.setInt(BbKey::kRecoveryCount, recovery_count);
    }
}

BtRoundRobinNode::BtRoundRobinNode(std::string name, std::vector<BtNodePtr> children)
    : name_(std::move(name))
    , children_(std::move(children))
{
}

NodeStatus BtRoundRobinNode::tick(BtBlackboard& /*blackboard*/)
{
    // Session 0 stub — Session 2 implements rotation semantics.
    active_child_name_.clear();
    return NodeStatus::Failure;
}

BtReactiveFallbackNode::BtReactiveFallbackNode(std::string name, std::vector<BtNodePtr> children)
    : name_(std::move(name))
    , children_(std::move(children))
{
}

NodeStatus BtReactiveFallbackNode::tick(BtBlackboard& /*blackboard*/)
{
    // Session 0 stub — Session 2 implements reactive interrupt semantics.
    active_child_name_.clear();
    return NodeStatus::Failure;
}

}  // namespace fleetsim::domain::behavior
