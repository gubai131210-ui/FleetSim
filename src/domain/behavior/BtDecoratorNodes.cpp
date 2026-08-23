#include "BtDecoratorNodes.h"

namespace fleetsim::domain::behavior {

BtRateDecoratorNode::BtRateDecoratorNode(std::string name, double hz, BtNodePtr child)
    : name_(std::move(name))
    , hz_(hz > 0.0 ? hz : 1.0)
    , child_(std::move(child))
{
}

NodeStatus BtRateDecoratorNode::tick(BtBlackboard& blackboard)
{
    blackboard.setString(BbKey::kActiveNodeName, name_);
    if (!child_) {
        return NodeStatus::Failure;
    }

    // Session 0 stub — always ticks child (no rate limiting yet).
    last_status_ = child_->tick(blackboard);
    return last_status_;
}

}  // namespace fleetsim::domain::behavior
