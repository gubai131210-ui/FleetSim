#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <variant>

namespace fleetsim::domain::behavior {

/// Shared key-value store for BT nodes (MVP — no port remapping).
class BtBlackboard {
public:
    using Value = std::variant<std::monostate, bool, int, double, std::string>;

    void setBool(const std::string& key, bool value) { storage_[key] = value; }
    void setInt(const std::string& key, int value) { storage_[key] = value; }
    void setDouble(const std::string& key, double value) { storage_[key] = value; }
    void setString(const std::string& key, const std::string& value) { storage_[key] = value; }

    void clearKey(const std::string& key) { storage_.erase(key); }
    void clear() { storage_.clear(); }

    std::optional<bool> getBool(const std::string& key) const;
    std::optional<int> getInt(const std::string& key) const;
    std::optional<double> getDouble(const std::string& key) const;
    std::optional<std::string> getString(const std::string& key) const;

    bool contains(const std::string& key) const;
    std::size_t size() const { return storage_.size(); }

private:
    std::unordered_map<std::string, Value> storage_;
};

}  // namespace fleetsim::domain::behavior
