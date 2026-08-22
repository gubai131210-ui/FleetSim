#include "MapLoader.h"

#include "MapSerializer.h"

namespace fleetsim::domain::map {

MapMetadata MapLoader::loadMetadataFromFile(const std::string& map_json_path)
{
    const MapDocument document = MapSerializer::fromFile(map_json_path);
    MapMetadata metadata;
    metadata.width_m = document.width_m;
    metadata.height_m = document.height_m;
    metadata.grid_resolution_m = document.grid_resolution_m;
    return metadata;
}

OccupancyGrid MapLoader::loadFromFile(const std::string& map_json_path,
                                      double inflation_radius_m)
{
    const MapDocument document = MapSerializer::fromFile(map_json_path);
    return MapSerializer::toOccupancyGrid(document, inflation_radius_m);
}

}  // namespace fleetsim::domain::map
