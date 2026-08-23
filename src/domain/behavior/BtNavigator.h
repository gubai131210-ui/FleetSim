#pragma once

#include "BtBlackboard.h"
#include "BtNode.h"

#include <memory>
#include <optional>
#include <string>

namespace fleetsim::domain::behavior {

struct BtTickResult {
    NodeStatus status{NodeStatus::Failure};
    std::string active_node_name;
};

/// Holds loaded tree + blackboard; tick entry for SimEngine (Session 2+).
class BtNavigator {
public:
    BtNavigator() = default;

    bool loadFromJsonFile(const std::string& path);
    bool hasTree() const { return root_ != nullptr; }

    BtTickResult tick(BtBlackboard& blackboard);
    BtBlackboard& blackboard() { return blackboard_; }
    const BtBlackboard& blackboard() const { return blackboard_; }

    const BtNode* root() const { return root_.get(); }
    std::string treeName() const { return tree_name_; }

private:
    BtNodePtr root_;
    BtBlackboard blackboard_;
    std::string tree_name_;
    NodeStatus last_status_{NodeStatus::Failure};
};

}  // namespace fleetsim::domain::behavior
