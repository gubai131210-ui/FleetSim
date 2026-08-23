#pragma once

#include <string>
#include <vector>

namespace fleetsim::domain::map {

struct LaneNode {
    std::string id;
    double x{0.0};
    double y{0.0};
};

struct LaneEdge {
    std::string from;
    std::string to;
    bool bidirectional{false};
};

struct LaneMapData {
    std::vector<LaneNode> nodes;
    std::vector<LaneEdge> edges;
};

using LanePath = std::vector<std::string>;

}  // namespace fleetsim::domain::map
