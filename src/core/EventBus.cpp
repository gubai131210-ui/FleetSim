#include "EventBus.h"

#include <algorithm>

namespace fleetsim::core {

int EventBus::subscribe(const std::string& topic, Handler handler)
{
    const int id = next_id_++;
    subscriptions_[topic].push_back(Subscription{id, std::move(handler)});
    return id;
}

void EventBus::unsubscribe(const std::string& topic, int subscription_id)
{
    auto topic_it = subscriptions_.find(topic);
    if (topic_it == subscriptions_.end()) {
        return;
    }

    auto& handlers = topic_it->second;
    handlers.erase(
        std::remove_if(
            handlers.begin(),
            handlers.end(),
            [subscription_id](const Subscription& sub) {
                return sub.id == subscription_id;
            }),
        handlers.end());
}

void EventBus::publish(const std::string& topic, const std::string& payload)
{
    const auto topic_it = subscriptions_.find(topic);
    if (topic_it == subscriptions_.end()) {
        return;
    }

    for (const Subscription& sub : topic_it->second) {
        if (sub.handler) {
            sub.handler(payload);
        }
    }
}

void EventBus::clear()
{
    subscriptions_.clear();
    next_id_ = 1;
}

}  // namespace fleetsim::core
