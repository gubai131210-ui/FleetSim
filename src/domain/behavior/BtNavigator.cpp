#include "BtNavigator.h"

#include "BtTreeLoader.h"

namespace fleetsim::domain::behavior {

bool BtNavigator::loadFromJsonFile(const std::string& path)
{
    BtTreeLoadError error;
    auto loaded = BtTreeLoader::loadFromFile(path, &error);
    if (!loaded.has_value()) {
        root_.reset();
        tree_name_.clear();
        return false;
    }

    root_ = std::move(loaded.value());
    tree_name_ = root_ ? root_->name() : std::string{};
    last_status_ = NodeStatus::Failure;
    return root_ != nullptr;
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
