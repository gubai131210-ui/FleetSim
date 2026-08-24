#include "domain/behavior/MultiBtNavigator.h"

namespace fleetsim::domain::behavior {

MultiBtNavigator::AgentEntry& MultiBtNavigator::entryFor(const std::string& agent_id)
{
    auto& entry = agents_[agent_id];
    entry.blackboard.setString(BbKey::kAgentId, agent_id);
    return entry;
}

const MultiBtNavigator::AgentEntry& MultiBtNavigator::entryFor(const std::string& agent_id) const
{
    return agents_.at(agent_id);
}

BtNavigator& MultiBtNavigator::navigatorFor(const std::string& agent_id)
{
    return entryFor(agent_id).navigator;
}

const BtNavigator& MultiBtNavigator::navigatorFor(const std::string& agent_id) const
{
    return entryFor(agent_id).navigator;
}

BtBlackboard& MultiBtNavigator::blackboardFor(const std::string& agent_id)
{
    return entryFor(agent_id).blackboard;
}

const BtBlackboard& MultiBtNavigator::blackboardFor(const std::string& agent_id) const
{
    return entryFor(agent_id).blackboard;
}

bool MultiBtNavigator::hasAgent(const std::string& agent_id) const
{
    return agents_.find(agent_id) != agents_.end();
}

void MultiBtNavigator::clear()
{
    agents_.clear();
    default_tree_path_.clear();
    default_tree_format_ = "json";
}

void MultiBtNavigator::setDefaultTree(const std::string& path, const std::string& format)
{
    default_tree_path_ = path;
    default_tree_format_ = format.empty() ? "json" : format;
}

bool MultiBtNavigator::loadTreeForAgent(const std::string& agent_id,
                                        const std::string& path,
                                        const std::string& format)
{
    AgentEntry& entry = entryFor(agent_id);
    entry.blackboard.setString(BbKey::kAgentId, agent_id);
    return entry.navigator.loadFromFile(path, format);
}

bool MultiBtNavigator::loadDefaultTreeForAgent(const std::string& agent_id)
{
    if (default_tree_path_.empty()) {
        return false;
    }
    return loadTreeForAgent(agent_id, default_tree_path_, default_tree_format_);
}

BtTickResult MultiBtNavigator::tickAgent(const std::string& agent_id,
                                       IBtSimContext& context,
                                       double replan_hz,
                                       int recovery_wait_ticks)
{
    AgentEntry& entry = entryFor(agent_id);
    entry.blackboard.setSimContext(&context);
    entry.blackboard.setString(BbKey::kAgentId, agent_id);
    entry.blackboard.setDouble(BbKey::kReplanHz, replan_hz);
    entry.blackboard.setInt(BbKey::kRecoveryWaitTicks, recovery_wait_ticks);
    return entry.navigator.tick(entry.blackboard);
}

}  // namespace fleetsim::domain::behavior
