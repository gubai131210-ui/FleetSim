#include "domain/behavior/BtXmlLoader.h"

namespace fleetsim::domain::behavior {

namespace {

constexpr const char* kSession0StubMessage =
    "BtXmlLoader not implemented (Phase 10 Session 2 pending)";

void setError(BtXmlLoadError* error, const std::string& message)
{
    if (error != nullptr) {
        error->message = message;
    }
}

}  // namespace

std::optional<BtNodePtr> BtXmlLoader::loadFromFile(const std::string& /*path*/,
                                                   BtXmlLoadError* error)
{
    setError(error, kSession0StubMessage);
    return std::nullopt;
}

std::optional<BtNodePtr> BtXmlLoader::loadFromXmlString(const std::string& /*xml*/,
                                                        BtXmlLoadError* error)
{
    setError(error, kSession0StubMessage);
    return std::nullopt;
}

}  // namespace fleetsim::domain::behavior
