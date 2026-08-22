#pragma once

#include "core/EventBus.h"

#include <memory>
#include <string>

namespace fleetsim::bridges::ros2 {

// Thin adapter: subscribe Domain EventBus string topics and (when rclcpp is
// linked) republish as std_msgs/String. Domain itself never includes rclcpp.
class Ros2Bridge {
public:
    explicit Ros2Bridge(core::EventBus& event_bus);
    ~Ros2Bridge();

    Ros2Bridge(const Ros2Bridge&) = delete;
    Ros2Bridge& operator=(const Ros2Bridge&) = delete;

    // Spin once / process pending callbacks. Safe no-op if ROS not initialized.
    void spinSome();

    bool isActive() const { return active_; }

private:
    core::EventBus& event_bus_;
    bool active_{false};
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace fleetsim::bridges::ros2
