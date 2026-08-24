#pragma once

#include "BtNode.h"

#include <memory>
#include <optional>
#include <string>

namespace fleetsim::domain::behavior {

struct BtXmlLoadError {
    std::string message;
};

/// Nav2 / BT.CPP teaching-subset XML → in-memory tree (ADR-023).
/// Coexists with BtTreeLoader (JSON). Session 0: stub; Session 2 implements.
class BtXmlLoader {
public:
    static std::optional<BtNodePtr> loadFromFile(const std::string& path,
                                                 BtXmlLoadError* error = nullptr);
    static std::optional<BtNodePtr> loadFromXmlString(const std::string& xml,
                                                      BtXmlLoadError* error = nullptr);
};

}  // namespace fleetsim::domain::behavior
