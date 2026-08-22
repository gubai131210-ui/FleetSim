# Optional ROS2 bridge — only configured when FLEETSIM_BUILD_ROS2=ON.
#
# When ROS 2 is not found, we still create a no-op INTERFACE target so the
# option can be toggled without breaking the main Qt tree.

find_package(rclcpp QUIET)
find_package(std_msgs QUIET)

if(rclcpp_FOUND AND std_msgs_FOUND)
    add_library(fleetsim_ros2_bridge STATIC
        fleetsim_ros2_bridge.cpp
        fleetsim_ros2_bridge.h
    )
    target_compile_definitions(fleetsim_ros2_bridge PUBLIC FLEETSIM_HAS_RCLCPP=1)
    target_link_libraries(fleetsim_ros2_bridge
        PUBLIC
            FleetSimDomain
            FleetSimCore
            rclcpp::rclcpp
            ${std_msgs_TARGETS}
    )
    target_include_directories(fleetsim_ros2_bridge PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    message(STATUS "FleetSim: ROS2 bridge ENABLED")
else()
    message(WARNING "FLEETSIM_BUILD_ROS2=ON but rclcpp/std_msgs not found — bridge skipped")
endif()
