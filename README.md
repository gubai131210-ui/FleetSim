# FleetSim

多 AGV 调度与控制系统 2D 仿真平台（C++17 + Qt 6 Widgets）。

## 文档

- [AGENTS.md](AGENTS.md) — Agent 协作规范
- [docs/DEVELOPMENT_PLAN.md](docs/DEVELOPMENT_PLAN.md) — 阶段计划
- [docs/decisions/](docs/decisions/) — 架构决策（ADR）
- [SESSION_LOG.md](SESSION_LOG.md) — 会话记录

## 构建

在 Qt Creator 中打开 `CMakeLists.txt`，配置 Kit（Qt 6.11+ MinGW），Build。

首次配置会通过 FetchContent 下载 Google Test、nlohmann/json、Eigen（需网络）。

## 当前阶段

**Phase 0** — 工程骨架、模块化 CMake、SVG 车辆渲染 ADR。

## 目录

```
src/core/      — SimClock, EventBus, 基础类型（无 Qt）
src/domain/    — SimEngine, 调度/避碰 stub（无 Qt）
src/app/       — SimController
src/ui/        — MainWindow, MapView, VehicleGraphicsItem（SVG）
assets/        — 车辆 SVG、场景 JSON
tests/         — Google Test
```
