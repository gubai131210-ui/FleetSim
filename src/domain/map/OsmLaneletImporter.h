#pragma once

#include "LaneTypes.h"

#include <optional>
#include <string>

namespace fleetsim::domain::map {

struct OsmImportError {
    std::string message;
};

/// OSM/Lanelet2 teaching subset → LaneMapData (ADR-022).
/// Session 0: interface + stub; Session 1 implements parsing.
class OsmLaneletImporter {
public:
    static std::optional<LaneMapData> importFromFile(const std::string& path,
                                                     OsmImportError* error = nullptr);
    static std::optional<LaneMapData> importFromXmlString(const std::string& xml,
                                                          OsmImportError* error = nullptr);
};

}  // namespace fleetsim::domain::map
