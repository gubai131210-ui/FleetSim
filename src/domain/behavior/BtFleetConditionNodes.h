#pragma once

#include "BtNode.h"

#include <string>

namespace fleetsim::domain::behavior {

class BtIsGoalUpdatedNode final : public BtNode {
public:
    explicit BtIsGoalUpdatedNode(std::string name);

    NodeStatus tick(BtBlackboard& blackboard) override;
    std::string name() const override { return name_; }
    BtNodeType type() const override { return BtNodeType::Condition; }

private:
    std::string name_;
};

class BtIsPathValidNode final : public BtNode {
public:
    explicit BtIsPathValidNode(std::string name);

    NodeStatus tick(BtBlackboard& blackboard) override;
    std::string name() const override { return name_; }
    BtNodeType type() const override { return BtNodeType::Condition; }

private:
    std::string name_;
};

class BtYieldIfBlockedNode final : public BtNode {
public:
    explicit BtYieldIfBlockedNode(std::string name);

    NodeStatus tick(BtBlackboard& blackboard) override;
    std::string name() const override { return name_; }
    BtNodeType type() const override { return BtNodeType::Condition; }

private:
    std::string name_;
};

}  // namespace fleetsim::domain::behavior
