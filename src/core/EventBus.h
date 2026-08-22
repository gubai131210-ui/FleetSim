#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace fleetsim::core {

class EventBus {
public:
    using Handler = std::function<void(const std::string& payload)>;

    int subscribe(const std::string& topic, Handler handler);
    void unsubscribe(const std::string& topic, int subscription_id);
    void publish(const std::string& topic, const std::string& payload = {});

    void clear();

private:
    struct Subscription {
        int id{0};
        Handler handler;
    };

    int next_id_{1};
    std::unordered_map<std::string, std::vector<Subscription>> subscriptions_;
};

}  // namespace fleetsim::core
