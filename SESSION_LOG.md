# FleetSim Session Log

## 会话流程

1. 读 `AGENTS.md` + `docs/AGENT_SESSION_TEMPLATE.md` + 本文件最新条目
2. Architect → mini-plan（含 NOT DO 列表）
3. Implementer → 代码 + 单测
4. Reviewer → 硬规则 + UI_GUIDELINES + 偷懒自检
5. Scribe → **按模板完整填写**下方新条目
6. 回复用户：做了什么 / 没做什么 / 为什么 / 本地验证

---

---

## [2026-08-23] Phase 4 Session 1 — BicycleModel 实装 + Vehicle 策略切换

### 本次 Scope
- 目标：后轴 Bicycle 运动学绿测；Vehicle `unique_ptr<IVehicleModel>`；工厂 diff_drive|bicycle；DiffDrive 默认不破
- 允许：`BicycleModel.*`、`Vehicle.*`、`VehicleModelFactory.*`、CMake、`SimController`、测试、SESSION_LOG
- 不做：舵角适配、bicycle 资产、Settings、Hungarian、ROS2、export

### ✅ 已完成
- [x] Bicycle 后轴方程 + clamp + L≤0 防护
- [x] Vehicle unique_ptr + 双构造 + setModel
- [x] VehicleModelFactory / createVehicleModel
- [x] SimController 按 model 工厂创建
- [x] BicycleModelTest / VehicleTest bicycle 注入

### ❌ 未完成 / 故意不做
| 项目 | 原因 | 计划 |
|------|------|------|
| Pure Pursuit→舵角 | Session 2 | Session 2 |
| bicycle_demo 资产 | Session 2 | Session 2 |
| Domain export / ROS2 / Settings / Hungarian | 后续 | Session 3–5 |

### Reviewer 结果
- Reviewer-Code: PASS / Reviewer-Test: PASS / Reviewer-UI: N/A

### 用户本地验证
1. `git pull` → Configure → Build → Run `FleetSimTests`
2. 预期：`BicycleModelTest.*` / `VehicleTest.*` / DiffDrive / MultiAgv **全 PASS**

---

## [2026-08-23] Phase 4 Session 2 — 舵角适配 + scenario bicycle + 场景资产

### 本次 Scope
- 目标：κ→δ=`atan(κL)`；scenario `wheelbase_m`；`bicycle_demo`；集成测；DiffDrive 路径不破
- 不做：Settings、Hungarian、ROS2、CMake export、Phase4 ✅

### ✅ 已完成
- [x] `SteeringAdapter`（atan + clamp）
- [x] `PurePursuitTracker` bicycle 重载填 `steering_angle`；大航向误差 creep+满舵
- [x] Vehicle 运动学元数据；SimEngine 按 isBicycle 分流
- [x] Scenario 读写 `wheelbase_m` / `max_steering_rad`
- [x] `assets/scenarios/bicycle_demo/`
- [x] `SteeringAdapterTest` + `BicycleScenarioTest`

### ❌ 未完成 / 故意不做
| 项目 | 原因 | 计划 |
|------|------|------|
| Domain 静态库 export | Session 3 | Session 3 |
| ROS2 桥 | Session 4 | Session 4 |
| SettingsDialog / Hungarian | Session 5 | Session 5 |
| Stanley tracker | 非 MVP | 后续可选 |

### 🚫 禁止偷懒自检
- [x] Bicycle ≠ DiffDrive 换皮
- [x] Domain 无 Qt/rclcpp
- [x] 未堆 Settings 进 ControlPanel
- [x] CMake include/link + 新源登记
- [x] 新类有单测；SESSION_LOG 写了没做什么

### Reviewer 结果
- Reviewer-Code: PASS / Reviewer-Test: PASS / Reviewer-UI: N/A

### 用户本地验证
1. `git pull` → Configure → Build → Run Tests（含 SteeringAdapter* / BicycleScenario*）
2. Open `assets/scenarios/bicycle_demo` → Play，观察转弯与 DiffDrive 差异

### 下次会话建议
- Session 3：CMake export FleetSimDomain + 无 Qt smoke

---

## [2026-08-23] Phase 4 Session 0 — ADR + IVehicleModel / ControlCommand stub + 红灯测

### 本次 Scope（Architect 定义）
- 目标：固化 ADR-009/010；扩展 `ControlCommand.steering_angle`；`IVehicleModel` + `BicycleModel` stub（运动学未实装，单测 RED）；`DiffDriveModel` 实现接口且行为不变
- 允许改动：`docs/decisions/009-*`、`010-*`、`src/core/types/ControlCommand.h`、`src/domain/vehicle/`、`src/domain/CMakeLists.txt`、`tests/`、`SESSION_LOG.md`
- 明确不在本次范围：完整 Bicycle 方程、Vehicle 策略切换、舵角适配、scenario 资产、CMake export、ROS2 桥代码、SettingsDialog、Hungarian、Phase4 ✅

### ✅ 已完成
- [x] ADR-009 `docs/decisions/009-bicycle-control-command.md`
- [x] ADR-010 `docs/decisions/010-ros2-bridge.md`
- [x] `ControlCommand.steering_angle`（默认 0）
- [x] `IVehicleModel.h`
- [x] `DiffDriveModel : public IVehicleModel`（行为不变）
- [x] `BicycleModel` stub（故意不积分，保持 RED）
- [x] `BicycleModelTest.cpp` 红灯用例 + CMake 登记

### ❌ 未完成 / 故意不做
| 项目 | 原因 | 计划在哪个 Phase/会话 |
|------|------|----------------------|
| Bicycle 后轴运动学实装 | Session 0 仅 stub+红灯 | Session 1 |
| Vehicle unique_ptr 模型切换 | 依赖完整 Bicycle | Session 1 |
| Pure Pursuit→舵角 / scenario bicycle | 后续 | Session 2 |
| Domain 静态库 export | 后续 | Session 3 |
| ROS2 桥代码 | 仅 ADR 草案 | Session 4 |
| SettingsDialog / Hungarian | 后续 | Session 5 |

### 🚫 禁止偷懒自检
- [x] 没有把 Bicycle stub 冒充完整运动学绿测
- [x] Domain 无 Qt / rclcpp
- [x] 未堆 Settings 进 ControlPanel
- [x] 未削掉 CMake target_include / link
- [x] 新 Domain 类有 GTest（红灯预期）
- [x] SESSION_LOG 已写「没做什么」

### 新增/变更文件清单
| 文件 | 操作 | 说明 |
|------|------|------|
| `docs/decisions/009-bicycle-control-command.md` | 新增 | ADR-009 |
| `docs/decisions/010-ros2-bridge.md` | 新增 | ADR-010 |
| `src/core/types/ControlCommand.h` | 修改 | +steering_angle |
| `src/domain/vehicle/IVehicleModel.h` | 新增 | 策略接口 |
| `src/domain/vehicle/BicycleModel.*` | 新增 | stub |
| `src/domain/vehicle/DiffDriveModel.h` | 修改 | 实现接口 |
| `src/domain/CMakeLists.txt` | 修改 | 登记 Bicycle |
| `tests/domain/BicycleModelTest.cpp` | 新增 | 红灯 |
| `tests/CMakeLists.txt` | 修改 | 登记测试 |

### 接口变更
- `ControlCommand` 新增 `steering_angle`（默认 0，兼容）
- 新增 `IVehicleModel::integrate(pose, cmd, dt)`
- `DiffDriveModel` / `BicycleModel` 实现该接口

### Reviewer 结果
- Reviewer-Code: PASS（Bicycle 真 stub、CMake include/link 保留、无 Qt/rclcpp）
- Reviewer-Test: PASS（五类用例齐；运动学用例 Session0 预期 RED）
- Reviewer-UI: N/A（本会话无 UI 改动）

### 用户本地验证
1. `git pull origin main`
2. Qt Creator 重新 Configure → Build
3. Run `FleetSimTests`：既有 DiffDrive/MultiAgv 应 PASS；`BicycleModelTest.*` **预期 FAIL**（Session 0 红灯）
4. Session 1 实装后上述 Bicycle 用例应变绿

### 下次会话建议
- 第一条任务：实现 `BicycleModel` 后轴方程 + Vehicle 策略切换 + 绿测
- 前置条件：本会话已 push；用户确认旧测仍绿、Bicycle 红灯可接受

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

## [2026-08-22] Phase 2 复盘 + Phase 3 启动备忘

### Phase 2 编译/集成问题（Phase 3 已对策）
- App 层裸 `map::`/`scenario::` → 一律 `domain::map::` / `domain::scenario::`
- `.h` 缺声明 → 接口先于实现 + Reviewer 对齐 grep
- Lambda 漏 capture → UI Review 必查
- QCustomPlot MinGW → `-Wa,-mbig-obj`
- MainWindow 膨胀 → Phase 3 用 `FleetUiCoordinator`

---

## [2026-08-22] Phase 3 — 多 AGV + 贪心调度 + 时间窗口避碰

### 本次 Scope
- 目标：FleetManager、GreedyAssigner、TimeWindow 避碰、TaskPanel、VehicleInfoPanel、multi_agv 场景
- 允许改动：`src/core/types/`、`src/domain/`、`src/app/`、`src/ui/`、`tests/`、`assets/scenarios/multi_agv/`、`docs/`

### ✅ 已完成
- [x] Core：`Task`、`TimeWindow`、`VehicleState`
- [x] Domain：`TaskQueue`、`GreedyAssigner`、`FleetManager`、`PathReservationTable`、`TimeWindowCollisionAvoidance`
- [x] `SimEngine` 多车 tick + 任务状态机（Pickup → Dropoff）
- [x] `ScenarioSerializer` tasks[] 读写；`assets/scenarios/multi_agv/`
- [x] App：`FleetUiCoordinator`、`SimController::addTask/selectVehicle`
- [x] UI：`TaskPanel`、`VehicleInfoPanel` 独立 Dock
- [x] 单测 + `MultiAgvScenarioTest`；ADR-008；ADR-003 状态更新

### ❌ 未完成 / 故意不做
| 项目 | 原因 | 计划 |
|------|------|------|
| HungarianAssigner | Phase 3 MVP 用 Greedy | Phase 3+ |
| ROS2 / Bicycle | Phase 4 | Phase 4 |
| 本地全量 Build 验证 | 中文路径由用户执行 | 用户 Qt Creator |

### 🚫 禁止偷懒自检
- [x] 未用简单距离判碰代替时间窗口
- [x] TaskPanel/VehicleInfoPanel 独立 Dock
- [x] Domain 无 Qt include
- [x] 新 Domain 类有单测
- [x] SESSION_LOG 已填写

### 用户本地验证
1. Build + Run Tests（含 MultiAgvScenarioTest）
2. File → Open → `assets/scenarios/multi_agv` → Play → 观察两车接任务、分色路径
3. Task 面板 Add Task；点击车辆查看 Vehicle Info

---

## [2026-08-22] 框架强化 — Agent 反偷懒 + UI 面板规范

（略，见历史条目）

## [2026-08-22] Phase 0 — Harness 与工程骨架

（略，见历史条目）
