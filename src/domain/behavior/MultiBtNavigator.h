#pragma once

#include "BtBlackboard.h"
#include "BtNavigator.h"
#include "BtTypes.h"
#include "IBtSimContext.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace fleetsim::domain::behavior {

/// Per-agent BtNavigator + Blackboard registry (ADR-025).
class MultiBtNavigator {
public:
    BtNavigator& navigatorFor(const std::string& agent_id);
    const BtNavigator& navigatorFor(const std::string& agent_id) const;
    BtBlackboard& blackboardFor(const std::string& agent_id);
    const BtBlackboard& blackboardFor(const std::string& agent_id) const;

    bool hasAgent(const std::string& agent_id) const;
    std::size_t agentCount() const { return agents_.size(); }

    void clear();
    void setDefaultTree(const std::string& path, const std::string& format);
    bool loadTreeForAgent(const std::string& agent_id,
                          const std::string& path,
                          const std::string& format);
    bool loadDefaultTreeForAgent(const std::string& agent_id);

    BtTickResult tickAgent(const std::string& agent_id,
                           IBtSimContext& context,
                           double replan_hz,
                           int recovery_wait_ticks);

    const std::string& defaultTreePath() const { return default_tree_path_; }
    const std::string& defaultTreeFormat() const { return default_tree_format_; }

private:
    struct AgentEntry {
        BtNavigator navigator;
        BtBlackboard blackboard;
    };

    AgentEntry& entryFor(const std::string& agent_id);
    const AgentEntry& entryFor(const std::string& agent_id) const;

    std::unordered_map<std::string, AgentEntry> agents_;
    std::string default_tree_path_;
    std::string default_tree_format_{"json"};
};

}  // namespace fleetsim::domain::behavior
