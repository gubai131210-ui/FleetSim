#include "fleetsim_ros2_bridge.h"

#if defined(FLEETSIM_HAS_RCLCPP)
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#endif

namespace fleetsim::bridges::ros2 {

struct Ros2Bridge::Impl {
#if defined(FLEETSIM_HAS_RCLCPP)
    rclcpp::Node::SharedPtr node;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pose_pub;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr path_pub;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr task_pub;
#endif
};

Ros2Bridge::Ros2Bridge(core::EventBus& event_bus)
    : event_bus_(event_bus)
    , impl_(std::make_unique<Impl>())
{
#if defined(FLEETSIM_HAS_RCLCPP)
    if (!rclcpp::ok()) {
        int argc = 0;
        char** argv = nullptr;
        rclcpp::init(argc, argv);
    }
    impl_->node = std::make_shared<rclcpp::Node>("fleetsim_bridge");
    impl_->pose_pub = impl_->node->create_publisher<std_msgs::msg::String>("fleetsim/pose", 10);
    impl_->path_pub = impl_->node->create_publisher<std_msgs::msg::String>("fleetsim/path", 10);
    impl_->task_pub =
        impl_->node->create_publisher<std_msgs::msg::String>("fleetsim/task_status", 10);

    event_bus_.subscribe("sim/pose_updated", [this](const std::string& payload) {
        std_msgs::msg::String msg;
        msg.data = payload;
        impl_->pose_pub->publish(msg);
    });
    event_bus_.subscribe("sim/path_updated", [this](const std::string& payload) {
        std_msgs::msg::String msg;
        msg.data = payload;
        impl_->path_pub->publish(msg);
    });
    event_bus_.subscribe("sim/task_updated", [this](const std::string& payload) {
        std_msgs::msg::String msg;
        msg.data = payload;
        impl_->task_pub->publish(msg);
    });
    active_ = true;
#else
    (void)event_bus_;
    active_ = false;
#endif
}

Ros2Bridge::~Ros2Bridge() = default;

void Ros2Bridge::spinSome()
{
#if defined(FLEETSIM_HAS_RCLCPP)
    if (active_ && impl_ && impl_->node) {
        rclcpp::spin_some(impl_->node);
    }
#endif
}

}  // namespace fleetsim::bridges::ros2
