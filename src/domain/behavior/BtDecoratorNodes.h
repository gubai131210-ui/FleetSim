#pragma once

#include "BtNode.h"

#include <string>

namespace fleetsim::domain::behavior {

/// Rate-limited decorator — ticks child at most once per 1/hz seconds (ADR-020).
class BtRateDecoratorNode final : public BtNode {
public:
    BtRateDecoratorNode(std::string name, double hz, BtNodePtr child);

    NodeStatus tick(BtBlackboard& blackboard) override;
    std::string name() const override { return name_; }
    BtNodeType type() const override { return BtNodeType::Rate; }
    std::string activeChildName() const override { return child_ ? child_->name() : std::string{}; }

    double hz() const { return hz_; }

private:
    std::string name_;
    double hz_{1.0};
    BtNodePtr child_;
    int cooldown_ticks_remaining_{0};
    NodeStatus last_status_{NodeStatus::Success};
};

}  // namespace fleetsim::domain::behavior
