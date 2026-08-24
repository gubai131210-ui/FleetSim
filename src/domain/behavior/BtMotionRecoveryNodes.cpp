#include "domain/behavior/BtMotionRecoveryNodes.h"

#include "BtTypes.h"
#include "IBtSimContext.h"

#include <algorithm>
#include <cmath>

namespace fleetsim::domain::behavior {

namespace {

IBtSimContext* requireContext(BtBlackboard& blackboard)
{
    return blackboard.simContext();
}

constexpr double kDefaultSpinOmegaRadS = 2.0;

}  // namespace

BtSpinRecoveryNode::BtSpinRecoveryNode(std::string name, double spin_rad)
    : name_(std::move(name))
    , spin_rad_(spin_rad)
{
}

NodeStatus BtSpinRecoveryNode::tick(BtBlackboard& blackboard)
{
    blackboard.setString(BbKey::kActiveNodeName, name_);
    IBtSimContext* context = requireContext(blackboard);
    if (context == nullptr) {
        return NodeStatus::Failure;
    }

    const double dt = context->simDt();
    if (dt <= 0.0) {
        return NodeStatus::Failure;
    }

    if (!active_) {
        active_ = true;
        accumulated_rad_ = 0.0;
    }

    const double target = std::abs(spin_rad_);
    const double sign = spin_rad_ >= 0.0 ? 1.0 : -1.0;
    const double remaining = std::max(0.0, target - accumulated_rad_);
    const double step = std::min(remaining, kDefaultSpinOmegaRadS * dt);

    context->applyYawDelta(sign * step);
    accumulated_rad_ += step;

    if (accumulated_rad_ + 1e-9 >= target) {
        active_ = false;
        accumulated_rad_ = 0.0;
        return NodeStatus::Success;
    }
    return NodeStatus::Running;
}

BtBackUpRecoveryNode::BtBackUpRecoveryNode(std::string name,
                                           double backup_dist_m,
                                           double backup_speed_mps)
    : name_(std::move(name))
    , backup_dist_m_(backup_dist_m)
    , backup_speed_mps_(backup_speed_mps)
{
}

NodeStatus BtBackUpRecoveryNode::tick(BtBlackboard& blackboard)
{
    blackboard.setString(BbKey::kActiveNodeName, name_);
    IBtSimContext* context = requireContext(blackboard);
    if (context == nullptr || backup_dist_m_ <= 0.0 || backup_speed_mps_ <= 0.0) {
        return NodeStatus::Failure;
    }

    const double dt = context->simDt();
    if (dt <= 0.0) {
        return NodeStatus::Failure;
    }

    if (!active_) {
        active_ = true;
        accumulated_dist_m_ = 0.0;
    }

    const double remaining = std::max(0.0, backup_dist_m_ - accumulated_dist_m_);
    const double step = std::min(remaining, backup_speed_mps_ * dt);
    context->applyBodyTranslation(-step, 0.0);
    accumulated_dist_m_ += step;

    if (accumulated_dist_m_ + 1e-9 >= backup_dist_m_) {
        active_ = false;
        accumulated_dist_m_ = 0.0;
        return NodeStatus::Success;
    }
    return NodeStatus::Running;
}

BtClearInflationNode::BtClearInflationNode(std::string name)
    : name_(std::move(name))
{
}

NodeStatus BtClearInflationNode::tick(BtBlackboard& blackboard)
{
    blackboard.setString(BbKey::kActiveNodeName, name_);
    IBtSimContext* context = requireContext(blackboard);
    if (context == nullptr) {
        return NodeStatus::Failure;
    }

    const std::size_t before = context->occupiedCellCount();
    const bool cleared = context->clearInflationLayer();
    blackboard.setBool(BbKey::kInflationCleared, cleared);
    if (!cleared) {
        return NodeStatus::Failure;
    }
    const std::size_t after = context->occupiedCellCount();
    return after <= before ? NodeStatus::Success : NodeStatus::Failure;
}

}  // namespace fleetsim::domain::behavior
