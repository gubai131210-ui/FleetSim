#pragma once

#include "BtNode.h"

#include <string>

namespace fleetsim::domain::behavior {

/// In-place spin recovery — integrates yaw toward target (ADR-023).
class BtSpinRecoveryNode final : public BtNode {
public:
    explicit BtSpinRecoveryNode(std::string name, double spin_rad = 1.5707963267948966);

    NodeStatus tick(BtBlackboard& blackboard) override;
    std::string name() const override { return name_; }
    BtNodeType type() const override { return BtNodeType::Action; }

    double spinRad() const { return spin_rad_; }

private:
    std::string name_;
    double spin_rad_{0.0};
};

/// Straight-line backup along vehicle rear heading (ADR-023).
class BtBackUpRecoveryNode final : public BtNode {
public:
    BtBackUpRecoveryNode(std::string name, double backup_dist_m = 0.3, double backup_speed_mps = 0.1);

    NodeStatus tick(BtBlackboard& blackboard) override;
    std::string name() const override { return name_; }
    BtNodeType type() const override { return BtNodeType::Action; }

private:
    std::string name_;
    double backup_dist_m_{0.0};
    double backup_speed_mps_{0.0};
};

/// Teaching ClearCostmap — clears inflation layer or obstacle cache before replan (ADR-023).
class BtClearInflationNode final : public BtNode {
public:
    explicit BtClearInflationNode(std::string name);

    NodeStatus tick(BtBlackboard& blackboard) override;
    std::string name() const override { return name_; }
    BtNodeType type() const override { return BtNodeType::Action; }

private:
    std::string name_;
};

}  // namespace fleetsim::domain::behavior
