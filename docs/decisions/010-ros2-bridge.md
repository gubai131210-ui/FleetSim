# ADR 010: ROS2 桥接（可选编译）

## 状态

已接受（2026-08-23，Phase 4 Session 0 草案；Session 4 实装）

## 背景

ADR-001 要求 Domain 零 Qt，可导出静态库接 ROS2。主工程用户可能无 ROS 环境，故桥接必须可选。

## 决策

1. **Domain / Core 禁止** `#include <rclcpp/*>` 与 Qt；桥不进入 `FleetSimDomain`
2. 桥目录：`bridges/ros2/`（独立 CMake / ament 包）
3. 主工程：
   ```cmake
   option(FLEETSIM_BUILD_ROS2 "Build ROS2 bridge" OFF)
   ```
   默认 **OFF**；无 ROS 仍可编 Qt 仿真
4. 通信：桥订阅 Domain `EventBus` 字符串 topic，或薄适配层将 JSON 发布到 ROS2 topic
5. MVP topic：
   - `fleetsim/pose`（或 per-vehicle）
   - `fleetsim/path`
   - `fleetsim/task_status`
   - Payload：`std_msgs/String` JSON（与现有 EventBus 一致）；后续可换自定义 msg
6. 文档：`docs/ros2_bridge.md`（Humble/Jazzy、Windows 限制）
7. Domain 导出：Session 3 完成 `install`/`export`；桥 `target_link_libraries(FleetSimDomain)`

## 不做

- 完整 Autoware / nav2
- Qt 与 ROS executor 同线程强耦合
- 把 ROS 依赖写进 Domain CMake

## 后果

- 正面：无 ROS 用户不受影响；有 ROS 可增量启用
- 负面：双构建系统（CMake Qt + ament）需文档维护

## 参考

- ROS2 C++ library tutorial
- `docs/PHASE4_GOAL_PROMPT.md` §4.4 / §5
