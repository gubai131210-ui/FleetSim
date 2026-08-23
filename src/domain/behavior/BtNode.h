#pragma once

#include "BtBlackboard.h"
#include "BtTypes.h"

#include <memory>
#include <string>
#include <vector>

namespace fleetsim::domain::behavior {

/// Abstract behavior tree node (ADR-020).
class BtNode {
public:
    virtual ~BtNode() = default;

    virtual NodeStatus tick(BtBlackboard& blackboard) = 0;
    virtual std::string name() const = 0;
    virtual BtNodeType type() const = 0;

    /// Last ticked child name for monitoring (control nodes override).
    virtual std::string activeChildName() const { return {}; }
};

using BtNodePtr = std::unique_ptr<BtNode>;

}  // namespace fleetsim::domain::behavior
