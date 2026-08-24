#pragma once

#include <cstdint>
#include <string>

namespace fleetsim::domain::behavior {

/// Node tick result — aligned with BehaviorTree.CPP / Nav2 semantics.
enum class NodeStatus : std::uint8_t {
    Success = 0,
    Failure = 1,
    Running = 2,
};

enum class BtNodeType : std::uint8_t {
    Sequence,
    Fallback,
    Recovery,
    Rate,
    RoundRobin,
    ReactiveFallback,
    Action,
    Condition,
};

/// Well-known blackboard keys (MVP).
namespace BbKey {
inline constexpr const char* kAgentId = "agent_id";
inline constexpr const char* kGoalUpdated = "goal_updated";
inline constexpr const char* kPathValid = "path_valid";
inline constexpr const char* kReplanRequested = "replan_requested";
inline constexpr const char* kRecoveryCount = "recovery_count";
inline constexpr const char* kActiveNodeName = "active_node_name";
inline constexpr const char* kReplanHz = "replan_hz";
inline constexpr const char* kRecoveryWaitTicks = "recovery_wait_ticks";
inline constexpr const char* kInflationCleared = "inflation_cleared";
}  // namespace BbKey

/// FleetSim registered action / condition node names (JSON "name" field).
namespace BtActionName {
inline constexpr const char* kPlanPath = "PlanPath";
inline constexpr const char* kFollowUntilGoal = "FollowUntilGoal";
inline constexpr const char* kWaitRecovery = "WaitRecovery";
inline constexpr const char* kReplanIfTimer = "ReplanIfTimer";
inline constexpr const char* kSpin = "Spin";
inline constexpr const char* kBackUp = "BackUp";
inline constexpr const char* kClearInflation = "ClearInflation";
}  // namespace BtActionName

namespace BtConditionName {
inline constexpr const char* kIsGoalUpdated = "IsGoalUpdated";
inline constexpr const char* kIsPathValid = "IsPathValid";
inline constexpr const char* kYieldIfBlocked = "YieldIfBlocked";
}  // namespace BtConditionName

inline const char* nodeStatusToString(NodeStatus status)
{
    switch (status) {
    case NodeStatus::Success:
        return "SUCCESS";
    case NodeStatus::Failure:
        return "FAILURE";
    case NodeStatus::Running:
        return "RUNNING";
    }
    return "UNKNOWN";
}

inline const char* btNodeTypeToString(BtNodeType type)
{
    switch (type) {
    case BtNodeType::Sequence:
        return "Sequence";
    case BtNodeType::Fallback:
        return "Fallback";
    case BtNodeType::Recovery:
        return "Recovery";
    case BtNodeType::Rate:
        return "Rate";
    case BtNodeType::RoundRobin:
        return "RoundRobin";
    case BtNodeType::ReactiveFallback:
        return "ReactiveFallback";
    case BtNodeType::Action:
        return "Action";
    case BtNodeType::Condition:
        return "Condition";
    }
    return "Unknown";
}

}  // namespace fleetsim::domain::behavior
