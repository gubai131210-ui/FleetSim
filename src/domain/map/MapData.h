#pragma once

#include "LaneTypes.h"

#include <string>
#include <utility>
#include <vector>

namespace fleetsim::domain::map {

enum class ObstacleType {
    Rect,
    Polygon,
};

struct RectObstacle {
    double x{0.0};
    double y{0.0};
    double width{0.0};
    double height{0.0};
};

struct PolygonObstacle {
    std::vector<std::pair<double, double>> vertices;
};

struct Obstacle {
    ObstacleType type{ObstacleType::Rect};
    RectObstacle rect;
    PolygonObstacle polygon;
};

struct MapDocument {
    int version{1};
    double width_m{20.0};
    double height_m{15.0};
    double grid_resolution_m{0.1};
    std::vector<Obstacle> obstacles;
    LaneMapData lanes;
};

}  // namespace fleetsim::domain::map
