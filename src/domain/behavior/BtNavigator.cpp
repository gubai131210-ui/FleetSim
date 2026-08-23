#include "BtNavigator.h"

namespace fleetsim::domain::behavior {

bool BtNavigator::loadFromJsonFile(const std::string& /*path*/)
{
    // Session 0 stub — Session 2 implements BtTreeLoader.
    root_.reset();
    tree_name_.clear();
    return false;
}

BtTickResult BtNavigator::tick(BtBlackboard& blackboard)
{
    BtTickResult result;
    result.status = NodeStatus::Failure;
    result.active_node_name.clear();

    if (!root_) {
        last_status_ = NodeStatus::Failure;
        return result;
    }

    result.status = root_->tick(blackboard);
    result.active_node_name = blackboard.getString(BbKey::kActiveNodeName).value_or(root_->name());
    last_status_ = result.status;
    return result;
}

}  // namespace fleetsim::domain::behavior
