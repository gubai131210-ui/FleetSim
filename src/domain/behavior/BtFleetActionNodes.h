#pragma once

#include "BtNode.h"

#include <string>

namespace fleetsim::domain::behavior {

class BtPlanPathNode final : public BtNode {
public:
    explicit BtPlanPathNode(std::string name);

    NodeStatus tick(BtBlackboard& blackboard) override;
    std::string name() const override { return name_; }
    BtNodeType type() const override { return BtNodeType::Action; }

private:
    std::string name_;
};

class BtFollowUntilGoalNode final : public BtNode {
public:
    explicit BtFollowUntilGoalNode(std::string name);

    NodeStatus tick(BtBlackboard& blackboard) override;
    std::string name() const override { return name_; }
    BtNodeType type() const override { return BtNodeType::Action; }

private:
    std::string name_;
};

class BtWaitRecoveryNode final : public BtNode {
public:
    explicit BtWaitRecoveryNode(std::string name);

    NodeStatus tick(BtBlackboard& blackboard) override;
    std::string name() const override { return name_; }
    BtNodeType type() const override { return BtNodeType::Action; }

private:
    std::string name_;
    int ticks_remaining_{-1};
};

class BtReplanIfTimerNode final : public BtNode {
public:
    explicit BtReplanIfTimerNode(std::string name);

    NodeStatus tick(BtBlackboard& blackboard) override;
    std::string name() const override { return name_; }
    BtNodeType type() const override { return BtNodeType::Action; }

private:
    std::string name_;
    int cooldown_ticks_remaining_{0};
};

}  // namespace fleetsim::domain::behavior
