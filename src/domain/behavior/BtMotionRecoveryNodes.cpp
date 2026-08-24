#include "domain/behavior/BtMotionRecoveryNodes.h"

namespace fleetsim::domain::behavior {

BtSpinRecoveryNode::BtSpinRecoveryNode(std::string name, double spin_rad)
    : name_(std::move(name))
    , spin_rad_(spin_rad)
{
}

NodeStatus BtSpinRecoveryNode::tick(BtBlackboard& /*blackboard*/)
{
    // Session 0 stub — Session 3 wires real kinematics.
    return NodeStatus::Failure;
}

BtBackUpRecoveryNode::BtBackUpRecoveryNode(std::string name,
                                           double backup_dist_m,
                                           double backup_speed_mps)
    : name_(std::move(name))
    , backup_dist_m_(backup_dist_m)
    , backup_speed_mps_(backup_speed_mps)
{
}

NodeStatus BtBackUpRecoveryNode::tick(BtBlackboard& /*blackboard*/)
{
    return NodeStatus::Failure;
}

BtClearInflationNode::BtClearInflationNode(std::string name)
    : name_(std::move(name))
{
}

NodeStatus BtClearInflationNode::tick(BtBlackboard& /*blackboard*/)
{
    return NodeStatus::Failure;
}

}  // namespace fleetsim::domain::behavior
