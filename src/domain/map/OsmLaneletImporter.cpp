#include "domain/map/OsmLaneletImporter.h"

namespace fleetsim::domain::map {

namespace {

constexpr const char* kSession0StubMessage =
    "OsmLaneletImporter not implemented (Phase 10 Session 1 pending)";

void setError(OsmImportError* error, const std::string& message)
{
    if (error != nullptr) {
        error->message = message;
    }
}

}  // namespace

std::optional<LaneMapData> OsmLaneletImporter::importFromFile(const std::string& /*path*/,
                                                              OsmImportError* error)
{
    setError(error, kSession0StubMessage);
    return std::nullopt;
}

std::optional<LaneMapData> OsmLaneletImporter::importFromXmlString(const std::string& /*xml*/,
                                                                   OsmImportError* error)
{
    setError(error, kSession0StubMessage);
    return std::nullopt;
}

}  // namespace fleetsim::domain::map
