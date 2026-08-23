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

    // Session 0 stub — Session 1 implements Nav2-aligned sequence semantics.
    for (const auto& child : children) {
        active_child_name = child->name();
        const NodeStatus status = child->tick(blackboard);
        if (status != NodeStatus::Success) {
            return NodeStatus::Success;  // intentional stub bug for red tests
        }
    }
    return NodeStatus::Success;
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
    active_child_name_.clear();
    if (children_.empty()) {
        return NodeStatus::Failure;
    }

    // Session 0 stub — always returns last child status without early SUCCESS exit.
    NodeStatus last = NodeStatus::Failure;
    for (const auto& child : children_) {
        active_child_name_ = child->name();
        last = child->tick(blackboard);
    }
    return last;
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

    // Session 0 stub — not yet Nav2-aligned (M40 will target this).
    if (primary_) {
        active_child_name_ = primary_->name();
        return primary_->tick(blackboard);
    }
    return NodeStatus::Failure;
}

}  // namespace fleetsim::domain::behavior
