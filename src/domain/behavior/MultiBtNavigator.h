#pragma once

#include "BtBlackboard.h"
#include "BtNavigator.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace fleetsim::domain::behavior {

/// Per-agent BtNavigator + Blackboard registry (ADR-025).
/// Session 0: interface + stub registry; Session 4 wires SimEngine tick.
class MultiBtNavigator {
public:
    BtNavigator& navigatorFor(const std::string& agent_id);
    BtBlackboard& blackboardFor(const std::string& agent_id);

    bool hasAgent(const std::string& agent_id) const;
    std::size_t agentCount() const { return agents_.size(); }

private:
    struct AgentEntry {
        BtNavigator navigator;
        BtBlackboard blackboard;
    };

    std::unordered_map<std::string, AgentEntry> agents_;
};

}  // namespace fleetsim::domain::behavior
