#pragma once

#include "OccupancyGrid.h"

#include <string>

namespace fleetsim::domain::map {

struct MapMetadata {
    double width_m{0.0};
    double height_m{0.0};
    double grid_resolution_m{0.1};
};

class MapLoader {
public:
    static OccupancyGrid loadFromFile(const std::string& map_json_path,
                                      double inflation_radius_m = 0.0);

    static MapMetadata loadMetadataFromFile(const std::string& map_json_path);
};

}  // namespace fleetsim::domain::map
