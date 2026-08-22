# ROS2 Bridge (optional)

Phase 4 Session 4. **Default OFF** — main Qt build must succeed without ROS 2.

## CMake

```cmake
option(FLEETSIM_BUILD_ROS2 "Build ROS2 bridge" OFF)
```

Enable only when ROS 2 Humble/Jazzy toolchain is available:

```
-DFLEETSIM_BUILD_ROS2=ON
```

## Layout

```
bridges/ros2/
  CMakeLists.txt          # only entered when FLEETSIM_BUILD_ROS2=ON
  fleetsim_ros2_bridge.cpp
  README.md
```

Domain / Core remain free of `rclcpp` and Qt.

## MVP topics

| Topic | Payload |
|-------|---------|
| `fleetsim/pose` | `std_msgs/String` JSON `{id,x,y,theta}` |
| `fleetsim/path` | JSON waypoints |
| `fleetsim/task_status` | JSON task id/status |

## Windows note

Official ROS 2 Windows support is limited. Prefer WSL2 / Linux CI for the bridge. The main FleetSim Qt app does not require ROS.

## Build (Linux / WSL example)

```bash
source /opt/ros/humble/setup.bash
cmake -S . -B build -DFLEETSIM_BUILD_ROS2=ON
cmake --build build --target fleetsim_ros2_bridge
```
