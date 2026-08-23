#include "BtBlackboard.h"

namespace fleetsim::domain::behavior {

namespace {

template <typename T>
std::optional<T> getAs(const BtBlackboard::Value& value)
{
    if (const auto* held = std::get_if<T>(&value)) {
        return *held;
    }
    return std::nullopt;
}

}  // namespace

std::optional<bool> BtBlackboard::getBool(const std::string& key) const
{
    const auto it = storage_.find(key);
    if (it == storage_.end()) {
        return std::nullopt;
    }
    return getAs<bool>(it->second);
}

std::optional<int> BtBlackboard::getInt(const std::string& key) const
{
    const auto it = storage_.find(key);
    if (it == storage_.end()) {
        return std::nullopt;
    }
    return getAs<int>(it->second);
}

std::optional<double> BtBlackboard::getDouble(const std::string& key) const
{
    const auto it = storage_.find(key);
    if (it == storage_.end()) {
        return std::nullopt;
    }
    return getAs<double>(it->second);
}

std::optional<std::string> BtBlackboard::getString(const std::string& key) const
{
    const auto it = storage_.find(key);
    if (it == storage_.end()) {
        return std::nullopt;
    }
    return getAs<std::string>(it->second);
}

bool BtBlackboard::contains(const std::string& key) const
{
    return storage_.find(key) != storage_.end();
}

}  // namespace fleetsim::domain::behavior
