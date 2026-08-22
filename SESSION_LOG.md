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

## [2026-08-22] Phase 2 — 地图编辑 + 持久化 + 监控 + 质量 Harness

### 本次 Scope
- 目标：MapEditor、ProjectManager/Serializer、MonitorPanel、CRAP Harness
- 允许改动：`src/app/`, `src/domain/`, `src/ui/`, `tests/`, `tools/`, `docs/`, `third_party/qcustomplot/`, `cmake/`

### ✅ 已完成
- [x] 会话0：`ENABLE_COVERAGE`、`tools/crap_score.py`、`tools/run_quality.ps1`、ADR-007、`MUTATION_CHECKLIST.md`
- [x] 会话1：`ProjectManager`、`MapSerializer`、`ScenarioSerializer` + 单测
- [x] 会话2：`MapEditorPanel`、`ObstacleGraphicsItem`、`MapView` 编辑模式、`MapScene::editorLayer_`
- [x] 会话3：`ProjectDialog`、File 菜单、去除启动 hardcode demo
- [x] 会话4：`MonitorPanel` + QCustomPlot、`MonitorBridge`、cross-track/velocity 曲线
- [x] 会话5：`LaneGraph` stub、SimEngine pose 发布 linear_velocity、SESSION_LOG/DEVELOPMENT_PLAN 更新

### ❌ 未完成 / 故意不做
| 项目 | 原因 | 计划 |
|------|------|------|
| Mull 自动变异 | MinGW/Clang Kit 冲突 | Phase 4 或 CI 评估 |
| 多车 TaskPanel | Phase 3 范围 | Phase 3 |
| 本地 CRAP 报告数值 | Agent 环境无 Qt Kit | 用户本地跑 `tools/run_quality.ps1` |

### 🚫 禁止偷懒自检
- [x] MapEditor / Monitor 独立 Dock，未堆进 MainWindow/ControlPanel
- [x] Domain Serializer 无 Qt include
- [x] 新 Serializer/ProjectManager 有单测
- [x] SESSION_LOG 已填写

### Reviewer 结果
- 待 Reviewer 子 agent

### 用户本地验证
1. Qt Creator Configure + Build + Run Tests（应含 MapSerializer/ScenarioSerializer/ProjectManager 测试）
2. Run → File → Open → `assets/scenarios/demo` → Editor 画矩形障碍 → File → Save → 重开验证
3. Editor 设 goal / Shift+点击 → Play → Monitor 面板曲线滚动
4. PowerShell：`./tools/run_quality.ps1` 生成 `build-quality/crap_report.txt`

---

## [2026-08-22] 框架强化 — Agent 反偷懒 + UI 面板规范

（略，见历史条目）

## [2026-08-22] Phase 0 — Harness 与工程骨架

（略，见历史条目）
