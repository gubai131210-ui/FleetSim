# Phase 5 Goal 模式执行提示词（完整版）

> **用途**：新开对话后，整段复制「§0 一键粘贴 Prompt」给 Agent（Goal 模式）。  
> **仓库**：https://github.com/gubai131210-ui/FleetSim.git  
> **本地路径**：`d:\QT_CppPrograms\FleetSim\FleetSim`  
> **前置**：Phase 0–4 已完成（Bicycle + Domain 静态库导出 + ROS2 可选桥 + SettingsDialog + Hungarian）。  
> **本阶段主题**：**非完整约束规划（Hybrid A*）+ Stanley 跟踪 + 可切换规划器/跟踪器 + 多车协调增强（Priority / CBS-lite）**。  
> **禁止**：编辑 `.cursor/plans/` 下的计划文件；Agent 不代跑中文用户 Temp 路径下的破坏性操作；中文路径 Qt Build 由用户本地验证（Agent 可在 **ASCII 工程路径** 的已有 Kit 上增量编译取证，但交付仍须附「用户必做 Build 步骤」）。

---

## §0 一键粘贴 Prompt（Goal 模式直接用）

```
你是 FleetSim Phase 5 的执行 Agent（Goal 模式）。
仓库：d:\QT_CppPrograms\FleetSim\FleetSim
远程：https://github.com/gubai131210-ui/FleetSim.git
权威说明：docs/PHASE5_GOAL_PROMPT.md（必须先完整阅读再写代码）

【强制阅读顺序 — 未读完禁止写代码】
1. docs/PHASE5_GOAL_PROMPT.md（全文，尤其 §1–§12）
2. docs/DEVELOPMENT_PLAN.md（确认 Phase 4 ✅、Phase 5 目标）
3. docs/AGENT_SESSION_TEMPLATE.md
4. SESSION_LOG.md（Phase 3/4 条目与防回归：CMake export、中文 Temp、Open Project 覆盖 model）
5. docs/decisions/001-architecture.md
6. docs/decisions/006-phase1-algorithms.md、009-bicycle-control-command.md、008-multi-vehicle-engine.md、003-collision-strategy.md
7. docs/UI_GUIDELINES.md + src/ui/panels/README.md
8. 代码优先：IPathPlanner / AStarPlanner / IPathTracker / PurePursuitTracker / SteeringAdapter /
   BicycleModel / Vehicle / SimEngine / TimeWindowCollisionAvoidance / PathReservationTable /
   SchedulingModule / SettingsDialog / 各层 CMakeLists / tests/CMakeLists.txt

【Phase 5 完成定义（全部达成才可结束 Goal）】
A. HybridAStarPlanner：状态 (x,y,θ)；按 Bicycle/最小转弯半径扩展；支持 Dubins（仅前进）MVP，Reeds-Shepp（可倒车）可选；不可把 A* 网格中心点当 Hybrid A*
B. scenario / Settings 可切换 planner: "astar"|"hybrid_astar"；bicycle 车默认 hybrid_astar（可配置）
C. StanleyTracker 实现 IPathTracker：前轴横向误差 + 航向误差；δ = θe + arctan(k e / v)；有单测；可与 PurePursuit 切换
D. SimEngine / Vehicle 按 tracker 类型注入；DiffDrive 仍可用 PurePursuit；回归不破
E. 多车协调增强：实现 PriorityPlanner 或 CBS-lite（至少一种）——禁止只改距离判碰；须与现有 TimeWindow / Reservation 协同或明确 ADR 说明替换边界
F. UI：独立 PlannerTrackerPanel 或扩展 SettingsDialog（禁止堆进 ControlPanel/MainWindow）；可切换 planner/tracker 并显示关键指标（横偏/航向误差）
G. CMake：新源登记；保留 target_include_directories / target_link_libraries；Domain 仍零 Qt、零 rclcpp
H. 文档：ADR-011（Hybrid A*）、ADR-012（Stanley）、ADR-013（多车协调增强，若做 CBS/Priority）；更新 DEVELOPMENT_PLAN Phase5✅、SESSION_LOG、MUTATION_CHECKLIST（M28+）
I. 测试：HybridAStarPlannerTest、StanleyTrackerTest、PlannerSwitchIntegrationTest 或扩展 BicycleScenarioTest；对比单测（Hybrid vs A* 在窄通道/最小转弯）；用户本地 Build + FleetSimTests 全绿才可 complete
J. 每会话 commit + push；子 Agent 双 Reviewer PASS；回复四段式

【架构硬约束】
UI → App → Domain → Core
Domain/Core 禁止 #include <Qt*> 与 rclcpp
App 层一律 domain::map:: / domain::scenario:: / domain::planning:: / domain::control:: / domain::scheduling::
接口先于实现：先 .h 后 .cpp；会话末 grep 头实现对齐
新 UI = 新 Panel/Dialog + 更新 CMakeLists；MainWindow 仅挂载 ≤15 行
禁止编辑 .cursor/plans/
中文路径：交付「用户必做 Build/测试步骤」；ProjectManager 类测试禁止依赖含中文的 %TEMP%（用仓库内 ASCII test_tmp）

【子 Agent 互相监督 — 每会话强制】
1. Architect：mini-plan（允许改动 / NOT DO / 文件清单）
2. Implementer：只按 mini-plan 实现
3. Reviewer-Code：对照禁止偷懒 + Phase2/3/4 防回归（命名空间、头同步、lambda、CMake include/link、nlohmann EXPORT）
4. Reviewer-Test：新类有单测/边界/集成；Hybrid「假实现」FAIL；缺失测 FAIL
5. Reviewer-UI：对照 UI_GUIDELINES；切换控件是否独立
6. Scribe：按 AGENT_SESSION_TEMPLATE 写 SESSION_LOG；回复四段式
任一 Reviewer FAIL → 不得声称完成，必须修复再审。主 Agent 禁止自评 PASS。

【建议 7 会话，可连续执行但禁止跳验收】
0: ADR-011/012/013 草案 + IPathPlanner/IPathTracker 扩展点调研 + 红灯测骨架
1: HybridAStarPlanner（Dubins MVP）+ HybridAStarPlannerTest
2: SimEngine/scenario 接入 hybrid_astar；bicycle 场景默认 Hybrid；对比测
3: StanleyTracker + StanleyTrackerTest；与 PurePursuit 切换
4: Priority 或多车 CBS-lite + 与 TimeWindow/Reservation 的 ADR 边界
5: PlannerTracker UI + Monitor 横偏/航向误差曲线
6: 端到端 + MUTATION + 双 Reviewer + push + Phase5 ✅

【禁止偷懒】
1. 禁止 Hybrid A* = 普通 A* 加个 θ 字段但不做运动学扩展
2. 禁止 Stanley = PurePursuit 换皮（必须用前轴横偏 + 航向项）
3. 禁止多车协调退化成「距离 < d 就停车」
4. 禁止 Domain 写 Qt / rclcpp
5. 禁止把 planner/tracker 切换堆进 ControlPanel/MainWindow
6. 禁止新类无 GTest；禁止跳过窄通道/最小转弯对比测
7. 禁止删掉 CMake target_include_directories / target_link_libraries
8. 禁止在 install(EXPORT) 里直接 PUBLIC 链接未导出的 FetchContent 目标（Phase4 nlohmann 教训：用 BUILD_INTERFACE + Config find_dependency）
9. 禁止跨阶段做完整 MPC / Autoware / 完整 nav2 / 完整最优 CBS（大规模）
10. 禁止 SESSION_LOG 不写「没做什么」
11. 禁止未更新 CMakeLists 就加文件
12. 禁止跳过双 Reviewer
13. 禁止破坏 Phase3 时间窗口避碰质量（除非 ADR-013 明确迁移并有回归测）
14. 禁止测试写入中文用户 Temp 导致 MinGW ofstream 假失败

【Todos】
按会话 0→6 创建/更新 todos；全部完成且 J 有测试绿证才 UpdateGoal complete。

从会话 0 开始立刻执行。中间不要换模型。
```

---

## §1 Phase 5 目标与验收标准

### 1.1 为何是 Phase 5（与学习路径对齐）

| 已完成 | Phase 5 要补的缺口 | 自动驾驶对应 |
|--------|-------------------|--------------|
| 栅格 A*（完整约束不足） | **Hybrid A***：`(x,y,θ)` + 运动学扩展 | Apollo/Autoware 开空间粗规划 |
| Pure Pursuit + κ→δ | **Stanley**：前轴横偏 + 航向 | DARPA Stanley 经典横向控制 |
| TimeWindow + Reservation | **Priority / CBS-lite** 显式冲突消解 | 仓储 MAPF / 协同 |
| Settings 模型/分配器 | **Planner/Tracker 策略切换 UI** | 算法对比实验台 |

官方计划在 Phase 4 后进入「乘用车级规划 + 更强跟踪 + 多车协同」；本阶段 **不做** 完整 MPC、完整 Autoware、完整最优大规模 CBS。

### 1.2 验收清单（全部勾选 = Phase 5 完成）

- [ ] `HybridAStarPlanner`：状态含朝向；按最小转弯半径 / bicycle 运动学生成后继；Dubins 解析扩展 MVP
- [ ] `scenario` / Settings：`planner: "astar"|"hybrid_astar"`；bicycle 场景可验证 Hybrid 优于裸 A*（窄弯/最小 R）
- [ ] `StanleyTracker : IPathTracker`；单测覆盖横偏收敛、航向项、低速保护
- [ ] `tracker: "pure_pursuit"|"stanley"` 可切换；DiffDrive 回归绿
- [ ] 多车：`PriorityPlanner` **或** `CbsLiteCoordinator`（二选一为 MVP，另一个可 stub+ADR）
- [ ] UI 独立面板/对话框；Monitor 增加 cross-track / heading error（若已有则接好数据源）
- [ ] ADR-011、ADR-012、（ADR-013）；DEVELOPMENT_PLAN Phase 5 ✅；SESSION_LOG；MUTATION M28+
- [ ] GTest + 用户本地 Build 全绿

### 1.3 明确不做（Phase 5 范围外）

| 项目 | 原因 |
|------|------|
| 完整 MPC 跟踪 | Phase 6+；算力与调参面过大 |
| Autoware / 完整 nav2 栈 | 部署与依赖爆炸 |
| 最优 CBS 大规模（数十车） | 教育向用 Priority 或 bounded CBS-lite |
| 动力学自行车（侧偏/轮胎力） | 仍用运动学 |
| 感知仿真（激光/相机） | 另阶段 |
| 把 Qt 搬进 Domain | 违反 ADR-001 |

---

## §2 当前框架结构（必须保持）

```
UI (Qt) → App (SimController / ProjectManager / FleetUiCoordinator)
        → Domain (SimEngine + planning/control/scheduling/collision/vehicle)
        → Core (Pose / Path / ControlCommand / EventBus / SimClock)

CMake:
FleetSimCore → FleetSimDomain → FleetSimApp → FleetSimUI → FleetSim
FleetSimTests → App + Domain + Core + GTest
bridges/ros2 可选（FLEETSIM_BUILD_ROS2=OFF）
examples/domain_smoke 无 Qt 链接 Domain
```

**Phase 5 增量（Domain）：**

```
planning/HybridAStarPlanner.*     (+ 可选 Dubins/ReedsShepp 工具)
control/StanleyTracker.*
scheduling 或 collision/
  PriorityPathCoordinator.*  或  CbsLiteCoordinator.*
```

**SimEngine 规划分支（示意）：**

```
if vehicle.isBicycle() && planner==hybrid_astar:
    HybridAStar.plan(start_pose_xyθ, goal, grid, wheelbase, max_steer)
else:
    AStar + Smoother   # 保持 Phase1–3 路径
→ tracker (PP | Stanley) → model.integrate
```

---

## §3 优先阅读的程序（按优先级）

### P0 — 动手前必读

| 文件 | 为什么 |
|------|--------|
| `src/domain/planning/IPathPlanner.h` / `AStarPlanner.*` | Hybrid 的接口对齐与对比基线 |
| `src/domain/planning/DouglasPeuckerSmoother.*` | Hybrid 输出是否还要平滑（ADR 决定） |
| `src/domain/control/IPathTracker.h` / `PurePursuitTracker.*` / `SteeringAdapter.*` | Stanley 并列策略 |
| `src/domain/vehicle/BicycleModel.*` / `Vehicle.*` | 最小转弯半径、wheelbase、isBicycle |
| `src/domain/SimEngine.cpp` | plan / tick 插入点 |
| `src/domain/collision/TimeWindowCollisionAvoidance.*` / `PathReservationTable.*` | 多车协调不得无脑替换 |
| `src/domain/scheduling/SchedulingModule.*` / `HungarianAssigner.*` | 任务层 vs 路径层职责分离 |
| `src/ui/dialogs/SettingsDialog.*` | 扩展或新建 PlannerTracker 对话框 |
| `CMakeLists` 各层 | **禁止削 include/link；EXPORT 用 BUILD_INTERFACE** |

### P1 — 测试与资产锚点

| 文件 | 为什么 |
|------|--------|
| `tests/domain/BicycleModelTest.cpp` / `BicycleScenarioTest.cpp` | 扩展 Hybrid/Stanley 集成 |
| `tests/domain/PurePursuitTrackerTest.cpp` / `SteeringAdapterTest.cpp` | Stanley 对照 |
| `tests/integration/MultiAgvScenarioTest.cpp` | 多车回归 |
| `assets/scenarios/bicycle_demo/` | 加窄弯/最小 R 地图变体 |
| `tools/verify_phase4_evidence.py` | 可仿写 `verify_phase5_evidence.py` |

### P2 — 防回归课（Phase 4）

| ID | 问题 | Phase 5 对策 |
|----|------|--------------|
| P4-EXPORT | nlohmann 进 install(EXPORT) 失败 | 新依赖同样 BUILD_INTERFACE + Config |
| P4-TEMP | 中文 Temp + MinGW ofstream | 测试用 `test_tmp/`（ASCII） |
| P4-OPEN | Open Project 覆盖 bicycle model | 加载保留 scenario；Settings 才 stamp |
| P3-CMake | 删光 include/link | 每次改 CMake 人工 diff |

---

## §4 网络调研摘要（执行 Agent 必读）

### 4.1 Hybrid A*（非完整约束）

**核心思想**：在离散搜索中用 **连续运动学** 扩展节点，状态为 `(x, y, θ)`（可再加档位），而不是栅格中心无朝向的 A*。

**典型步骤**（Apollo / MATLAB `plannerHybridAStar` 共识）：

1. 优先队列按 `f = g + h` 弹出节点  
2. 按离散舵角集合 + 前进（/倒车）用自行车模型积分生成后继，碰撞检测  
3. 周期性 **解析扩展**：Dubins（仅前进）或 Reeds-Shepp（可倒车）尝试直连目标；无碰则提前结束  
4. 启发式：无障碍欧氏/Reeds-Shepp 长度 + 可选障碍启发式（2D Dijkstra）

**Phase 5 MVP 建议**：

| 项 | 选择 |
|----|------|
| 解析曲线 | **先 Dubins**（实现简单）；Reeds-Shepp 标可选 |
| 输出 | `Path`（waypoints 带 θ 或另存 `PosePath`） |
| 与 A* | 并存；`planner` 字段切换 |
| 平滑 | Hybrid 已较平滑；Douglas-Peucker **可选关闭**（ADR 写明） |

**参考**：

- MATLAB Hybrid A*：https://www.mathworks.com/help/nav/ref/plannerhybridastar.html  
- Apollo Hybrid A* 文档：https://github.com/ApolloAuto/apollo/blob/master/docs/08_Planning/hybrid_a_star_en.md  
- arXiv 工业窄通道 Hybrid A*：https://arxiv.org/abs/2304.14043  
- ROS2 示例实现：https://github.com/armando-genis/hybrid_path_planning  

### 4.2 Stanley vs Pure Pursuit

| | Pure Pursuit | Stanley |
|--|--------------|---------|
| 参考点 | 后轴 + 前瞻点 | **前轴**最近点 |
| 误差 | 几何曲率 κ=2sin(α)/Ld | 航向误差 θe + 横偏 e |
| 舵角 | δ=arctan(κ L)（已有） | δ=θe + arctan(k e / (v+ε)) |
| 特点 | 简单稳、易切角 | 收敛快，对路径不连续敏感 |

**Phase 5**：实现经典 Stanley；低速 `v→0` 必须有 ε 保护；限舵与 Bicycle `max_steering` 一致。

**参考**：

- CMU Snider 技术报告（PP/Stanley）：https://publications.ri.cmu.edu/storage/publications/pub_files/2009/2/Automatic_Steering_Methods_for_Autonomous_Automobile_Path_Tracking.pdf  
- 对比综述（SAE / Sensors 混合控制）：几何控制器仍是教学与实车常用基线  

### 4.3 多车：Priority vs CBS

**CBS（Conflict-Based Search）**：高层约束树消冲突，底层单车 A*/时空 A*；最优但规模敏感。仓储常用 **ECBS/EECBS** 等有界次优变体加速。

**Priority-Based Planning**：按优先级依次规划，低优先级把高优先级轨迹当动态障碍；实现简单，适合 FleetSim 3–8 车教学规模。

**Phase 5 MVP 建议**：

1. **优先实现 `PriorityPathCoordinator`**：与现有 `PathReservationTable` / TimeWindow 对接  
2. CBS-lite：仅网格 + 点/边冲突 + 深度限制（可选第二会话）  
3. **禁止**用「距离 &lt; d 停车」冒充 MAPF  

**参考**：

- Sharon et al. CBS：Artificial Intelligence 2015  
- EECBS（AAAI）：有界次优加速  
- 仓储/码头 AGV 上的 bounded CBS 应用论文（CMES 等）  

### 4.4 ST-Graph（了解即可，本阶段不强制实现）

时空图把障碍预测轨迹填入 `(x,y,t)`，再沿参考线做速度规划。属 Phase 6 候选；Phase 5 若时间富余可只写 ADR 草稿，**禁止**未测完 Hybrid/Stanley 就开做 ST-Graph。

---

## §5 设计决策建议（先写 ADR 再写码）

### ADR-011 — Hybrid A*

1. `IPathPlanner` 保持；新增 `HybridAStarPlanner`  
2. 输入：`Pose start/goal`、`OccupancyGrid`、`wheelbase`、`max_steer`、`motion_resolution`  
3. Dubins 扩展间隔可配置（如每 N 次扩展尝试一次）  
4. bicycle 车辆默认 `hybrid_astar`；diff_drive 默认 `astar`  

### ADR-012 — Stanley

1. `StanleyTracker : IPathTracker`  
2. 输出 `ControlCommand`：填 `steering_angle`（bicycle）并可选填 `angular_velocity`（兼容）  
3. 参数：`k_gain`、`softening_v`、`max_steer`  

### ADR-013 — 多车路径协调

1. 任务分配仍 Hungarian/Greedy（Phase 4）  
2. 路径层：`PriorityPathCoordinator` 在 `planPathForAgent` 后写 Reservation  
3. 与 TimeWindow：保留速度缩放；Priority 负责「谁先占走廊」  

---

## §6 建议新增/变更文件清单

### Domain

| 文件 | 职责 |
|------|------|
| `planning/HybridAStarPlanner.h/.cpp` | 主规划器 |
| `planning/DubinsPath.h/.cpp` | Dubins 曲线（可先单文件） |
| `planning/ReedsSheppPath.*` | 可选 |
| `control/StanleyTracker.h/.cpp` | Stanley |
| `collision/PriorityPathCoordinator.*` 或 `scheduling/CbsLiteCoordinator.*` | 多车 |
| 扩展 `SimEngine` / scenario 字段 `planner`/`tracker` | 接线 |

### UI

| 文件 | 职责 |
|------|------|
| `dialogs/PlannerTrackerDialog.*` **或** 扩展 `SettingsDialog` | 切换算法（独立文件优先） |
| `MonitorPanel` / Bridge | 横偏、航向误差曲线 |

### 测试 / 资产 / 文档

```
tests/domain/HybridAStarPlannerTest.cpp
tests/domain/StanleyTrackerTest.cpp
tests/domain/PriorityCoordinatorTest.cpp   # 或 CbsLite
tests/integration/HybridBicycleScenarioTest.cpp
assets/scenarios/hybrid_narrow_turn/      # 小转弯半径挑战图
docs/decisions/011-hybrid-astar.md
docs/decisions/012-stanley-tracker.md
docs/decisions/013-priority-or-cbs-lite.md
tools/verify_phase5_evidence.py
```

---

## §7 如何继续开发（Goal 节奏）

1. `git pull`；确认 Phase 4：`FleetSimTests` 60 绿、bicycle_demo 可玩  
2. 会话 0：只 ADR + 接口 + 红灯测  
3. 会话 1–2：Hybrid 绿 → 接入引擎 → 场景可玩  
4. 会话 3：Stanley 绿 → Settings/Dialog 切换  
5. 会话 4：Priority 或 CBS-lite + 多车回归  
6. 会话 5–6：UI 指标 + 文档 + **双 Reviewer** + push + Phase5 ✅  
7. 每会话：SESSION_LOG 四段 + 用户验证清单  

### 子 Agent 监督协议

| 角色 | 输入 | 输出 | 否决权 |
|------|------|------|--------|
| Architect | Phase 目标 | mini-plan + NOT DO | 范围蔓延 |
| Implementer | mini-plan | 代码+测试 | — |
| Reviewer-Code | diff + §10 | PASS/FAIL | FAIL 阻 complete |
| Reviewer-Test | tests/ | 覆盖缺口 | 无单测即 FAIL |
| Reviewer-UI | UI diff | 一功能一面板 | FAIL |
| Scribe | 全会话 | SESSION_LOG + 四段式 | 模板不全=未完成 |

**主 Agent 不得自评 PASS**；用 Task 拉 `explore`/`generalPurpose` 做 Reviewer。

---

## §8 Phase 2/3/4 防回归（继续生效）

| ID | 对策 |
|----|------|
| P2-01 App 裸命名空间 | 一律 `domain::` 前缀 |
| P2-02 头实现不同步 | 先 .h 后 .cpp |
| P3-CMake | 改完 diff `target_*` |
| P4-EXPORT | FetchContent 依赖勿直接 EXPORT |
| P4-TEMP | 测试 ASCII `test_tmp` |
| P4-OPEN | 勿让默认 Settings 覆盖 scenario model |
| P3-避碰 | 禁止距离判碰替换 TimeWindow（除非 ADR-013+回归） |

---

## §9 关键参数默认值

| 参数 | 默认 | 说明 |
|------|------|------|
| hybrid `min_turning_radius` | L / tan(δmax) | 与 bicycle 一致 |
| hybrid 舵角离散 | 3～5 档 | [-δmax,0,δmax] 可再加密 |
| hybrid 解析扩展间隔 | 每 5～10 节点 | Dubins |
| stanley `k` | 1.0～2.5 | 横偏增益 |
| stanley `ε` | 0.1 m/s | 低速软化 |
| priority | 车 id 序或任务优先级 | 可配置 |
| planner 默认 | bicycle→hybrid_astar；diff→astar | scenario 可覆盖 |

---

## §10 禁止偷懒清单（Reviewer 打印打勾）

1. 禁止 Hybrid A* = A* 换皮  
2. 禁止 Stanley = PurePursuit 换皮  
3. 禁止多车协调 = 简单距离停车  
4. 禁止 Domain 含 Qt / rclcpp  
5. 禁止切换 UI 塞进 ControlPanel  
6. 禁止新 Domain 类无 GTest  
7. 禁止削 CMake `target_include_directories` / `target_link_libraries`  
8. 禁止 EXPORT 未导出的 FetchContent 目标  
9. 禁止完整 MPC / Autoware / 大规模最优 CBS  
10. 禁止 SESSION_LOG 缺「没做什么」  
11. 禁止不加 CMakeLists 就加源文件  
12. 禁止跳过双 Reviewer  
13. 禁止破坏 TimeWindow 避碰质量（无 ADR）  
14. 禁止中文 Temp 单测假红  

---

## §11 测试策略（最低标准）

| 层级 | 要求 |
|------|------|
| 单元 | Hybrid：直道、最小 R 弯、无解/超时；Dubins 长度；Stanley：e→0 趋势、ε 保护、限舵 |
| 对比 | 同图 bicycle：Hybrid 成功而「A*+强制 θ」失败或代价更差（构造窄弯） |
| 回归 | DiffDrive、MultiAgv、Hungarian、BicycleModel、SteeringAdapter、domain_smoke 全绿 |
| 集成 | hybrid 场景 load→plan→N tick→姿态沿弯道；切换 Stanley 后横偏曲线下降 |
| 多车 | Priority 后无顶点冲突或 TimeWindow 缩放仍触发；禁止静默互相穿过 |
| 突变 | M28 Hybrid 扩展步长符号；M29 Stanley k 取反；M30 Priority 忽略高优先级轨迹 |

---

## §12 用户本地验证清单（每次交付必须附上）

1. `git pull origin main`  
2. Qt Creator：**重新 Configure** → Build（MinGW Debug）  
3. 运行 `FleetSimTests`（关注 Hybrid* / Stanley* / Bicycle* / MultiAgv* / ProjectManager*）  
4. 可选：`pwsh -File tools/run_phase4_verify.ps1` 或新建 `run_phase5_verify.ps1`  
5. Open `assets/scenarios/hybrid_narrow_turn`（或文档路径）→ Play  
6. Settings/PlannerDialog：切 `astar`/`hybrid_astar`、`pure_pursuit`/`stanley`，观察转弯与横偏  
7. 多车场景：确认无「穿模」；Reservation/TimeWindow 仍工作  
8. （可选）`fleet_domain_smoke` 仍链接成功  

---

## §13 风险与缓解

| 风险 | 缓解 |
|------|------|
| Hybrid 搜索爆炸 | 限制开集、解析扩展、栅格分辨率、超时返回失败 |
| Stanley 在路径折点振荡 | 路径插值；限舵；与 PP 切换对比 |
| CBS 太难 | MVP 用 Priority；CBS-lite 限深 |
| 破坏旧 A* 场景 | 默认 diff_drive 仍 A*；单测锁行为 |
| EXPORT 再挂掉 | 新第三方库只 BUILD_INTERFACE |

---

## §14 参考链接

1. https://www.mathworks.com/help/nav/ref/plannerhybridastar.html  
2. https://github.com/ApolloAuto/apollo/blob/master/docs/08_Planning/hybrid_a_star_en.md  
3. https://arxiv.org/abs/2304.14043  
4. https://publications.ri.cmu.edu/storage/publications/pub_files/2009/2/Automatic_Steering_Methods_for_Autonomous_Automobile_Path_Tracking.pdf  
5. Sharon et al., Conflict-based search for optimal multi-agent pathfinding, AI Journal 2015  
6. EECBS：https://ojs.aaai.org/index.php/AAAI/article/view/17466  
7. https://github.com/armando-genis/hybrid_path_planning  

---

## §15 新对话开场白（短版，可与 §0 连用）

> 执行 `docs/PHASE5_GOAL_PROMPT.md`。Goal：完成 Phase 5（Hybrid A* + Stanley + Priority/CBS-lite + 可切换 UI）。严格子 Agent 互相监督；足够 GTest；每会话 push；不改 plan 文件；中文路径 Build 交付验证步骤。从会话 0 开始，todos 标 in_progress/completed，直到验收清单全勾选。
