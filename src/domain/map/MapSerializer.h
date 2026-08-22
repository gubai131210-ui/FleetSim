#pragma once

#include "MapData.h"
#include "OccupancyGrid.h"

#include <nlohmann/json_fwd.hpp>

#include <string>

namespace fleetsim::domain::map {

class MapSerializer {
public:
    static MapDocument fromJson(const nlohmann::json& json);
    static MapDocument fromFile(const std::string& map_json_path);
    static nlohmann::json toJson(const MapDocument& document);

    static bool saveToFile(const MapDocument& document, const std::string& map_json_path);
    static OccupancyGrid toOccupancyGrid(const MapDocument& document, double inflation_radius_m = 0.0);

    static bool isValidRect(const RectObstacle& rect);
    static bool isValidPolygon(const PolygonObstacle& polygon);
};

}  // namespace fleetsim::domain::map
