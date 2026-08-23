#include "BtTreeLoader.h"

namespace fleetsim::domain::behavior {

std::optional<BtNodePtr> BtTreeLoader::loadFromFile(const std::string& /*path*/,
                                                      BtTreeLoadError* error)
{
    if (error != nullptr) {
        error->message = "BtTreeLoader not implemented (Phase 9 Session 2)";
    }
    return std::nullopt;
}

std::optional<BtNodePtr> BtTreeLoader::loadFromJsonString(const std::string& /*json*/,
                                                          BtTreeLoadError* error)
{
    if (error != nullptr) {
        error->message = "BtTreeLoader not implemented (Phase 9 Session 2)";
    }
    return std::nullopt;
}

}  // namespace fleetsim::domain::behavior
