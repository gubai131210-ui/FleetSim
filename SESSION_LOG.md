# FleetSim Session Log

## 会话流程

1. 读 `AGENTS.md` + `docs/AGENT_SESSION_TEMPLATE.md` + 本文件最新条目
2. Architect → mini-plan（含 NOT DO 列表）
3. Implementer → 代码 + 单测
4. Reviewer → 硬规则 + UI_GUIDELINES + 偷懒自检
5. Scribe → **按模板完整填写**下方新条目
6. 回复用户：做了什么 / 没做什么 / 为什么 / 本地验证

---

## [2026-08-23] Phase 5 Session 1 — HybridAStarPlanner + Dubins MVP

### 本次 Scope（Architect 定义）
- 目标：真 Hybrid A*（`(x,y,θ)` 自行车扩展 + Dubins 解析扩展）；`HybridAStarPlannerTest` / `DubinsPathTest` 转绿；Stanley stub 保持红灯
- 允许：`DubinsPath.*`、`HybridAStarPlanner.cpp`、相关测试与 CMake、SESSION_LOG、ADR-011 状态可标「实装中」
- NOT DO：SimEngine 接线、Stanley 真公式、UI、Priority、Reeds-Shepp、完整 CBS

### ✅ 已完成
- [x] `DubinsPath`：LSL/LSR/RSL/RSR/RLR/LRL + sample
- [x] `HybridAStarPlanner::plan`：舵角离散扩展、碰撞采样、visited `(ix,iy,θbin)`、周期性 Dubins、开集上限
- [x] `DubinsPathTest`；CMake 登记 Dubins
- [x] Session0 Hybrid 红灯测改为实现后应对齐（用户本地验证）

### ❌ 未完成 / 故意不做
| 项目 | 原因 | 计划 |
|------|------|------|
| SimEngine / scenario `planner` | Session 2 | Session 2 |
| Stanley 真实现 | Session 3 | Session 3 |
| Reeds-Shepp 倒车 | Phase5 可选 | 有需要再开 |
| Priority 协调 | Session 4 | Session 4 |

### 🚫 禁止偷懒自检
- [x] Hybrid ≠ A* 换皮（运动学扩展 + Dubins）
- [x] Domain 无 Qt
- [x] 新类有 GTest
- [x] CMake 仅追加，未削 include/link
- [x] 未堆 UI / 未改 `.cursor/plans/`
- [x] SESSION_LOG 写明没做什么

### 新增/变更文件清单
| 文件 | 操作 | 说明 |
|------|------|------|
| `src/domain/planning/DubinsPath.h/.cpp` | 新增 | Dubins |
| `src/domain/planning/HybridAStarPlanner.cpp` | 重写 | 真搜索 |
| `tests/domain/DubinsPathTest.cpp` | 新增 | |
| `src/domain/CMakeLists.txt` / `tests/CMakeLists.txt` | 修改 | 登记 |
| `SESSION_LOG.md` | 修改 | 本条目 |

### 接口变更
- 无公开 API 破坏；`HybridAStarPlanner::plan` 行为从空路径变为运动学路径

### Reviewer 结果
- Reviewer-Code: **PASS**（真运动学 + Dubins；非阻断：终点 snap / 负坐标 key）
- Reviewer-Test: 初审 FAIL（窄弯）→ 修 closed/走廊后复审 **PASS**
- 主 Agent 不自评

### 用户本地验证
1. `git pull origin main`
2. Qt Creator 重新 Configure → Build `FleetSimTests`
3. 过滤运行：`HybridAStar*`、`DubinsPath*` — 预期 PASS
4. `Stanley*` — 仍预期多项 FAIL（Session 3）
5. 旧测（AStar/Bicycle/MultiAgv）应仍绿

### 下次会话建议
- SimEngine + scenario `planner` 字段；bicycle 默认 `hybrid_astar`；窄弯对比集成

---
### 本次 Scope（Architect 定义）
- 目标：Phase 5 会话 0 — ADR-011/012/013 草案；IPathPlanner/IPathTracker 扩展点调研落地；Hybrid/Stanley stub + 红灯 GTest；CMake 登记
- 允许改动：`docs/decisions/011|012|013*.md`、`src/domain/planning/HybridAStar*`、`src/domain/control/Stanley*`、`tests/domain/*Hybrid*|Stanley*`、两处 CMakeLists、`SESSION_LOG.md`
- 明确不在本次范围：Hybrid/Stanley 真算法、SimEngine 接线、UI、Priority 实装、Phase5 ✅

### ✅ 已完成
- [x] ADR-011 Hybrid A*（接口保持、运动学扩展、跳过 DP、bicycle 默认 hybrid）
- [x] ADR-012 Stanley（前轴横偏+航向、ε、与 PP 并存）
- [x] ADR-013 Priority + TimeWindow 协同边界（CBS-lite 可选）
- [x] 调研结论写入 ADR：SimEngine 硬编码 A*+PP；Path 无 θ；scenario 尚无 planner/tracker
- [x] `HybridAStarPlanner` stub（空 plan；`minTurningRadiusM` 可用）
- [x] `StanleyTracker` stub（零命令）
- [x] `HybridAStarPlannerTest` / `StanleyTrackerTest` 红灯骨架（期望 stub 下多项 FAIL）
- [x] Domain/Tests CMakeLists 登记新源（保留 include/link）

### ❌ 未完成 / 故意不做
| 项目 | 原因 | 计划 |
|------|------|------|
| Hybrid 运动学搜索 + Dubins | 会话 1 | Session 1 |
| SimEngine/scenario planner 字段 | 会话 2 | Session 2 |
| Stanley 真公式 | 会话 3 | Session 3 |
| PriorityPathCoordinator | 会话 4 | Session 4 |
| PlannerTracker UI / Monitor 误差 | 会话 5 | Session 5 |
| Phase5 ✅ / MUTATION M28+ | 会话 6 | Session 6 |

### 🚫 禁止偷懒自检
- [x] 没有把多个类挤进同一文件
- [x] 没有在 MainWindow 堆业务控件
- [x] 无新 UI（本会话不做 UI）
- [x] Domain 层无 Qt / rclcpp
- [x] 新 Domain 类有对应单测（红灯骨架）
- [x] 未跨 Phase 写完整 Hybrid/Stanley/CBS
- [x] SESSION_LOG 本节完整
- [x] 未削 CMake target_include / target_link
- [x] 未碰 `.cursor/plans/`

### 新增/变更文件清单
| 文件 | 操作 | 说明 |
|------|------|------|
| `docs/decisions/011-hybrid-astar.md` | 新增 | ADR-011 |
| `docs/decisions/012-stanley-tracker.md` | 新增 | ADR-012 |
| `docs/decisions/013-priority-or-cbs-lite.md` | 新增 | ADR-013 |
| `src/domain/planning/HybridAStarPlanner.h/.cpp` | 新增 | Session0 stub |
| `src/domain/control/StanleyTracker.h/.cpp` | 新增 | Session0 stub |
| `tests/domain/HybridAStarPlannerTest.cpp` | 新增 | 红灯 |
| `tests/domain/StanleyTrackerTest.cpp` | 新增 | 红灯 |
| `src/domain/CMakeLists.txt` | 修改 | 登记源 |
| `tests/CMakeLists.txt` | 修改 | 登记测 |
| `SESSION_LOG.md` | 修改 | 本条目 |

### 接口变更
- 新增 `planning::HybridAStarPlanner : IPathPlanner`（构造注入 L/δmax；`plan` 签名不变）
- 新增 `control::StanleyTracker : IPathTracker`
- scenario / SimEngine **尚未**接线（Session 2/3）

### Reviewer 结果
- Reviewer-Code: **PASS**（Domain 零 Qt；CMake 仅追加源；stub 诚实；ADR 覆盖扩展点）
- Reviewer-Test: **PASS**（7 例预期红灯；`MinTurningRadiusMatchesBicycleGeometry` 可绿；OccupancyGrid/BicycleModel API 正确）
- Reviewer-UI: N/A（本会话无 UI）
- 主 Agent 不自评；以子 Agent 结论为准

### 用户本地验证
1. `git pull origin main`
2. Qt Creator 重新 Configure → Build FleetSimTests
3. 运行测试：预期 **Hybrid\*** / **Stanley\*** 多项 FAIL（红灯锚点）；其余旧测应仍绿；`MinTurningRadiusMatchesBicycleGeometry` 可 PASS
4. 确认 Domain 无 Qt include；CMake 仍有 `target_include_directories` / `target_link_libraries`

### 下次会话建议
- 第一条任务：实现 HybridAStarPlanner（自行车扩展 + Dubins MVP）使 Hybrid 单测转绿
- 前置条件：本会话 ADR-011 已接受；用户确认红灯测能编译链接

---

## [2026-08-23] Phase 4 Goal J — CMake export fix + full FleetSimTests green

### ✅ 已完成
- [x] 修复 install(EXPORT) 依赖未导出的 nlohmann_json（BUILD_INTERFACE + Config find_dependency）
- [x] 现有 Kit 增量 Configure/Build FleetSimTests 成功
- [x] fleet_domain_smoke 运行 OK
- [x] ProjectManagerTest：改用 ASCII cwd 下 test_tmp（规避中文 Temp + MinGW ofstream）
- [x] **FleetSimTests 全量 60/60 PASSED**（含 Bicycle*/Steering*/Hungarian*/MultiAgv*）

### ❌ 未完成 / 故意不做
| 项目 | 原因 |
|------|------|
| Stanley / MPC | Phase4 范围外 |

### 证据
- FleetSimTests.exe LastWrite 2026-08-23；FULL=0；60 passed
- domain_smoke: domain_smoke ok pose=(...)

### Reviewer
- Code: CMake export 修复必要；Test: 全绿


---

## [2026-08-23] Phase 4 — 测试资产 cwd + 本地验证脚本（Goal J 前置）

### 本次 Scope
- 修复：FleetSimTests 未拷贝 assets / 未设 WORKING_DIRECTORY → BicycleScenarioTest 易 SKIP
- 新增：	ools/run_phase4_verify.ps1（静态审计 + 拒绝陈旧 exe + 过滤跑测）

### ✅ 已完成
- [x] tests/CMakeLists.txt POST_BUILD copy assets + gtest WORKING_DIRECTORY=源码根
- [x] run_phase4_verify.ps1（检出 exe 早于 2026-08-23 则拒绝作为 J 证据）

### ❌ 未完成
| 项目 | 原因 | 计划 |
|------|------|------|
| Goal complete / J 运行时绿 | 现有 FleetSimTests.exe=2026-08-22 陈旧 | 用户 Qt 重编后跑 ps1 |

### 用户本地验证
1. git pull
2. Qt Creator 重新 Configure → Build
3. pwsh -File tools/run_phase4_verify.ps1
4. 全绿后回复：Build/Tests 通过


---

## [2026-08-23] Phase 4 audit fix — Open Project 不再覆盖 bicycle model

### 本次 Scope
- 修复：pplyProjectToSimulation 曾用默认 diff_drive Settings 覆盖 scenario 的 model=bicycle
- 增加：syncSettingsFromScenario（Open 后 Settings 反映场景）
- 增加：	ools/verify_phase4_evidence.py（A–J 静态证据审计，PASS=37）

### ✅ 已完成
- [x] Open Project 保留 bicycle_demo 模型
- [x] Settings 仍可主动 stamp + recreate
- [x] 静态审计脚本全绿（不含本机 Qt Build）

### ❌ 未完成 / 故意不做
| 项目 | 原因 | 计划 |
|------|------|------|
| 代跑 Qt Build / FleetSimTests | 中文路径禁止 Agent 代跑 | **用户本地** |
| Goal complete | 缺 J 运行时绿测证据 | 用户确认后关闭 |

### Reviewer
- 待 push 后 Code/Test 快审

### 用户本地验证（关 Goal 前置）
1. git pull
2. Qt Creator **重新 Configure** → Build（旧 FleetSimTests.exe 时间戳早于 Phase4，必须重编）
3. Run FleetSimTests（Bicycle*/Steering*/Hungarian*/MultiAgv*）
4. python tools/verify_phase4_evidence.py → PASS=37 FAIL=0
5. Open assets/scenarios/bicycle_demo → Play（须为 bicycle 转弯）
6. File→Settings 切 hungarian / model 后观察 recreate


---


## [2026-08-23] Phase 4 Sessions 3–5 — Domain export + ROS2 可选桥 + Settings/Hungarian + Phase4 ✅

### 本次 Scope
- Session3: FleetSimCore/Domain install+export + domain_smoke（无 Qt 链接）+ docs/domain_export.md
- Session4: bridges/ros2 + FLEETSIM_BUILD_ROS2=OFF + docs/ros2_bridge.md（ADR-010）
- Session5: SettingsDialog 独立对话框、HungarianAssigner、对比单测、DEVELOPMENT_PLAN Phase4✅、MUTATION M24–27、panels README

### ✅ 已完成
- [x] CMake export / install Domain+Core；.gitignore 白名单 cmake/*.cmake.in
- [x] examples/domain_smoke + FLEETSIM_BUILD_DOMAIN_SMOKE
- [x] ROS2 桥目录默认 OFF；无 rclcpp 时跳过不挂主工程
- [x] SettingsDialog + File→Settings（未堆 ControlPanel）
- [x] HungarianAssigner + SchedulingModule 可注入 + 对比单测
- [x] DEVELOPMENT_PLAN Phase4 ✅；MUTATION M24–27

### ❌ 未完成 / 故意不做
| 项目 | 原因 | 计划 |
|------|------|------|
| Stanley / MPC / Autoware | Phase4 明确不做 | 后续研究 |
| Windows 原生 ROS2 实机 publish | 环境限制；文档化 WSL | 有 ROS 的机器 |
| 用户本机 Qt Build 代跑 | 中文路径禁止 Agent 代跑 | 用户本地 |
| 时间窗口避碰改距离 | 禁止回退 | — |

### 🚫 禁止偷懒自检
- [x] Bicycle 非 DiffDrive 换皮（前序会话已落地）
- [x] Domain 无 Qt/rclcpp
- [x] Settings 独立 Dialog
- [x] 静态库 export 落地
- [x] ROS2 默认 OFF
- [x] 新类有单测；CMake include/link 保留
- [x] SESSION_LOG 写了没做什么

### Reviewer 结果
- Reviewer-Code: PASS（Settings 已回写 model/wheelbase 并 recreate）
- Reviewer-Test: PASS
- Reviewer-UI: PASS

### 用户本地验证
1. git pull；Qt Creator 重新 Configure → Build
2. Run FleetSimTests（Bicycle* / Steering* / Hungarian* / MultiAgv*）
3. 构建并运行 fleet_domain_smoke
4. Open assets/scenarios/bicycle_demo → Play
5. File → Settings 切换 assigner / model（reload 场景生效于车辆模型）
6. （可选）-DFLEETSIM_BUILD_ROS2=ON（需 ROS）


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
