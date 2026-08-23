#pragma once

#include "BtNode.h"

#include <memory>
#include <optional>
#include <string>

namespace fleetsim::domain::behavior {

struct BtTreeLoadError {
    std::string message;
};

/// JSON → in-memory tree (ADR-020 schema version 1). Session 2 implements parsing.
class BtTreeLoader {
public:
    static std::optional<BtNodePtr> loadFromFile(const std::string& path,
                                                 BtTreeLoadError* error = nullptr);
    static std::optional<BtNodePtr> loadFromJsonString(const std::string& json,
                                                       BtTreeLoadError* error = nullptr);
};

}  // namespace fleetsim::domain::behavior
