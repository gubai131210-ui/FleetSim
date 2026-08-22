# FleetSim Session Log

## 会话流程

1. 读 `AGENTS.md` + `docs/AGENT_SESSION_TEMPLATE.md` + 本文件最新条目
2. Architect → mini-plan（含 NOT DO 列表）
3. Implementer → 代码 + 单测
4. Reviewer → 硬规则 + UI_GUIDELINES + 偷懒自检
5. Scribe → **按模板完整填写**下方新条目
6. 回复用户：做了什么 / 没做什么 / 为什么 / 本地验证

---

## [2026-08-22] Phase 1 — 核心仿真 MVP

### 本次 Scope
- 目标：单车 A* + 平滑 + Pure Pursuit + DiffDrive + 场景加载 + UI 闭环
- 允许改动：`src/core/`, `src/domain/`, `src/app/`, `src/ui/`, `tests/`, `docs/decisions/006-*`

### ✅ 已完成
- [x] Core：`Waypoint`, `Path`, `GridCell`
- [x] Domain map：`OccupancyGrid`, `MapLoader`
- [x] Domain planning：`AStarPlanner`, `DouglasPeuckerSmoother`, 接口
- [x] Domain control：`PurePursuitTracker`, `IPathTracker`
- [x] Domain vehicle：`DiffDriveModel`, `Vehicle`
- [x] Domain scenario：`ScenarioLoader`
- [x] `SimEngine` 扩展：plan/tick/EventBus 发布
- [x] `SimController`：`loadScenario`, `setGoal`, `planPath`, `setTimeScale`
- [x] UI：`PathGraphicsItem`, `ObstacleOverlayItem`, MapScene 分层, MapView Shift+点击设目标
- [x] `MainWindow` 去 hardcode，QTimer 仿真循环，EventBus 订阅
- [x] `ControlPanel` 增加 Speed 1x/2x/4x
- [x] 单测 + `DemoScenarioTest` 集成测试
- [x] ADR-006 Phase 1 算法参数

### ❌ 未完成 / 故意不做
| 项目 | 原因 | 计划 |
|------|------|------|
| MonitorPanel | Phase 2 | Phase 2 |
| MapEditorPanel | Phase 2 | Phase 2 |
| JSON 写回 | Phase 2 ProjectManager | Phase 2 |
| 多车 | Phase 3 | Phase 3 |

### 🚫 禁止偷懒自检
- [x] 无 MainWindow 业务按钮堆叠
- [x] Domain 无 Qt include
- [x] 一文件一职责
- [x] A* 路径经 Smoother
- [x] 新 Domain 类有单测
- [x] SESSION_LOG 已填写

### Reviewer 结果
- 待 Reviewer 子 agent

### 用户本地验证
1. Qt Creator 重新 Configure + Build + Run Tests
2. Run → Shift+点击设目标 → Play → 小车沿绿线绕障运动

---

## [2026-08-22] 框架强化 — Agent 反偷懒 + UI 面板规范

（略，见历史条目）

## [2026-08-22] Phase 0 — Harness 与工程骨架

（略，见历史条目）
