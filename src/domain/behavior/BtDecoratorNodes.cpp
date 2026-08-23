#include "BtDecoratorNodes.h"

#include <algorithm>
#include <cmath>

namespace fleetsim::domain::behavior {

namespace {

int cooldownTicksForHz(double hz)
{
    if (hz <= 0.0) {
        return 1;
    }
    return std::max(1, static_cast<int>(std::lround(1.0 / hz)));
}

}  // namespace

BtRateDecoratorNode::BtRateDecoratorNode(std::string name, double hz, BtNodePtr child)
    : name_(std::move(name))
    , hz_(hz > 0.0 ? hz : 1.0)
    , child_(std::move(child))
    , cooldown_ticks_remaining_(0)
{
}

NodeStatus BtRateDecoratorNode::tick(BtBlackboard& blackboard)
{
    blackboard.setString(BbKey::kActiveNodeName, name_);
    if (!child_) {
        return NodeStatus::Failure;
    }

    if (cooldown_ticks_remaining_ > 0) {
        --cooldown_ticks_remaining_;
        return last_status_;
    }

    last_status_ = child_->tick(blackboard);
    cooldown_ticks_remaining_ = cooldownTicksForHz(hz_);
    return last_status_;
}

}  // namespace fleetsim::domain::behavior
