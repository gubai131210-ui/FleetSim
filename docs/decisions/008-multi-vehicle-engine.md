# ADR 008: 多车 SimEngine 模型

## 状态

已接受（2026-08-22，Phase 3 实现）

## 决策

- `FleetManager` 持有多 `VehicleAgent`（vehicle + path + goal + task phase）
- `SimEngine::tick` 编排：调度 → 规划 → 预约 → 避碰 → 跟踪积分
- 单车 API（`vehicle()`, `setGoal()`, `planPath()`）保留为**首车/选中车**兼容 wrapper
- EventBus：`sim/pose_updated` 含 `id`；`sim/path_updated` 含 `id` + waypoints
- 任务状态机：Idle → ToPickup → ToDropoff → Idle

## 后果

- Phase 1–2 单测与 demo 场景仍可用（单车）
- UI 通过 `FleetUiCoordinator` 绑定多车图元，MainWindow 不膨胀
