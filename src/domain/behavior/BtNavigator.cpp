#include "BtNavigator.h"

#include "BtTreeLoader.h"
#include "BtXmlLoader.h"

namespace fleetsim::domain::behavior {

bool BtNavigator::loadFromJsonFile(const std::string& path)
{
    return loadFromFile(path, "json");
}

bool BtNavigator::loadFromFile(const std::string& path, const std::string& format)
{
    BtNodePtr loaded;
    if (format == "xml") {
        BtXmlLoadError error;
        auto xml_loaded = BtXmlLoader::loadFromFile(path, &error);
        if (!xml_loaded.has_value()) {
            root_.reset();
            tree_name_.clear();
            return false;
        }
        loaded = std::move(xml_loaded.value());
    } else {
        BtTreeLoadError error;
        auto json_loaded = BtTreeLoader::loadFromFile(path, &error);
        if (!json_loaded.has_value()) {
            root_.reset();
            tree_name_.clear();
            return false;
        }
        loaded = std::move(json_loaded.value());
    }

    root_ = std::move(loaded);
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
