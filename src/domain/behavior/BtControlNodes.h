#pragma once

#include "BtNode.h"

#include <string>
#include <vector>

namespace fleetsim::domain::behavior {

class BtSequenceNode final : public BtNode {
public:
    explicit BtSequenceNode(std::string name, std::vector<BtNodePtr> children);

    NodeStatus tick(BtBlackboard& blackboard) override;
    std::string name() const override { return name_; }
    BtNodeType type() const override { return BtNodeType::Sequence; }
    std::string activeChildName() const override { return active_child_name_; }

private:
    std::string name_;
    std::vector<BtNodePtr> children_;
    std::string active_child_name_;
};

class BtFallbackNode final : public BtNode {
public:
    explicit BtFallbackNode(std::string name, std::vector<BtNodePtr> children);

    NodeStatus tick(BtBlackboard& blackboard) override;
    std::string name() const override { return name_; }
    BtNodeType type() const override { return BtNodeType::Fallback; }
    std::string activeChildName() const override { return active_child_name_; }

private:
    std::string name_;
    std::vector<BtNodePtr> children_;
    std::string active_child_name_;
    std::size_t running_child_index_{0};
    bool resume_running_child_{false};
};

/// Nav2-aligned RecoveryNode: primary child first; on FAILURE tick recovery child.
class BtRecoveryNode final : public BtNode {
public:
    BtRecoveryNode(std::string name, BtNodePtr primary, BtNodePtr recovery, int retries);

    NodeStatus tick(BtBlackboard& blackboard) override;
    std::string name() const override { return name_; }
    BtNodeType type() const override { return BtNodeType::Recovery; }
    std::string activeChildName() const override { return active_child_name_; }

    int retriesRemaining() const { return retries_remaining_; }

private:
    std::string name_;
    BtNodePtr primary_;
    BtNodePtr recovery_;
    int max_retries_{0};
    int retries_remaining_{0};
    std::string active_child_name_;
};

/// Nav2 RoundRobin: rotate child index; any SUCCESS → SUCCESS; all FAILURE → FAILURE.
class BtRoundRobinNode final : public BtNode {
public:
    explicit BtRoundRobinNode(std::string name, std::vector<BtNodePtr> children);

    NodeStatus tick(BtBlackboard& blackboard) override;
    std::string name() const override { return name_; }
    BtNodeType type() const override { return BtNodeType::RoundRobin; }
    std::string activeChildName() const override { return active_child_name_; }

    std::size_t currentChildIndex() const { return current_index_; }

private:
    std::string name_;
    std::vector<BtNodePtr> children_;
    std::size_t current_index_{0};
    std::string active_child_name_;
};

/// BT.CPP ReactiveFallback: re-evaluate from first child each tick; high-priority SUCCESS
/// can interrupt a RUNNING lower-priority child.
class BtReactiveFallbackNode final : public BtNode {
public:
    explicit BtReactiveFallbackNode(std::string name, std::vector<BtNodePtr> children);

    NodeStatus tick(BtBlackboard& blackboard) override;
    std::string name() const override { return name_; }
    BtNodeType type() const override { return BtNodeType::ReactiveFallback; }
    std::string activeChildName() const override { return active_child_name_; }

private:
    std::string name_;
    std::vector<BtNodePtr> children_;
    std::string active_child_name_;
};

}  // namespace fleetsim::domain::behavior
