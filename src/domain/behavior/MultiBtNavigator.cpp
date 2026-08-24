#include "domain/behavior/MultiBtNavigator.h"

namespace fleetsim::domain::behavior {

BtNavigator& MultiBtNavigator::navigatorFor(const std::string& agent_id)
{
    return agents_[agent_id].navigator;
}

BtBlackboard& MultiBtNavigator::blackboardFor(const std::string& agent_id)
{
    auto& entry = agents_[agent_id];
    entry.blackboard.setString(BbKey::kAgentId, agent_id);
    return entry.blackboard;
}

bool MultiBtNavigator::hasAgent(const std::string& agent_id) const
{
    return agents_.find(agent_id) != agents_.end();
}

}  // namespace fleetsim::domain::behavior
