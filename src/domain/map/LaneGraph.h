#pragma once

#include <string>
#include <vector>

namespace fleetsim::domain::map {

// Phase 2 stub — lane graph routing reserved for Phase 3+.
class LaneGraph {
public:
    bool loadFromFile(const std::string& /*map_json_path*/) { return true; }
    bool empty() const { return nodes_.empty(); }
    std::size_t nodeCount() const { return nodes_.size(); }

private:
    std::vector<int> nodes_;
};

}  // namespace fleetsim::domain::map
