# ADR 001: 整体架构

## 状态

已接受（2026-08-22）

## 背景

FleetSim 需要从 2D AGV 仿真逐步演进到自动驾驶算法研究，要求模块可持续扩展、Domain 可脱离 Qt 复用。

## 决策

采用四层架构 + 混合通信模式：

```
UI (Qt Widgets) → App (SimController) → Domain (SimEngine + 模块) → Core (类型/时钟/事件)
```

- **SimEngine tick 循环**：直接调用 Domain 模块（确定性、可测）
- **EventBus**：UI / Monitor 订阅状态变更（解耦）
- **Domain 零 Qt 依赖**：后期可编译为静态库接 ROS2

## CMake Target 链

```
FleetSimCore (STATIC) → FleetSimDomain (STATIC) → FleetSimApp (STATIC) → FleetSimUI (STATIC) → FleetSim (EXE)
FleetSimTests (EXE) → FleetSimDomain, FleetSimCore, GTest
```

## 后果

- 正面：测试可覆盖 Domain 而不启动 GUI；模块边界清晰
- 负面：初期 CMake 配置较默认模板复杂
