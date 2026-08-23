# Phase 6 Goal 模式执行提示词（完整版）

> **用途**：新开对话后，整段复制「§0 一键粘贴 Prompt」给 Agent（Goal 模式）。  
> **仓库**：https://github.com/gubai131210-ui/FleetSim.git  
> **本地路径**：`d:\QT_CppPrograms\FleetSim\FleetSim`  
> **前置**：Phase 0–5 ✅（Hybrid A* + Stanley + Priority + PlannerTrackerDialog；FleetSimTests 曾证 82 绿）。  
> **本阶段主题**：**教学向线性 MPC 横向跟踪** + **ST-Graph 纵向速度规划（沿参考路径）** + 与现有 PP/Stanley/Hybrid/Priority **可切换对比**。  
> **禁止**：编辑 `.cursor/plans/`；完整 Autoware / 完整 nav2 / 非线性大规模 MPC / 感知仿真；Agent 不代跑中文用户 Temp 破坏性操作；中文路径 Qt Build 由用户验证（Agent 可在 **ASCII** 路径如 `D:\build\FleetSim_*` 外置构建取证，交付仍须附「用户必做步骤」）。

---

## §0 一键粘贴 Prompt（Goal 模式直接用）

```
你是 FleetSim Phase 6 的执行 Agent（Goal 模式）。
仓库：d:\QT_CppPrograms\FleetSim\FleetSim
远程：https://github.com/gubai131210-ui/FleetSim.git
权威说明：docs/PHASE6_GOAL_PROMPT.md（必须先完整阅读再写代码）

【强制阅读顺序 — 未读完禁止写代码】
1. docs/PHASE6_GOAL_PROMPT.md（全文，尤其 §1–§12）
2. docs/DEVELOPMENT_PLAN.md、docs/learning-path.md（确认 Phase 5 ✅、Phase 6 目标）
3. docs/AGENT_SESSION_TEMPLATE.md
4. SESSION_LOG.md（Phase 5 Goal J 条目；防回归：CMake EXPORT、中文 Temp、Open Project 覆盖 model、Hybrid/Stanley 假实现）
5. docs/decisions/001-architecture.md、009-bicycle-control-command.md、011-hybrid-astar.md、012-stanley-tracker.md、013-priority-or-cbs-lite.md
6. docs/UI_GUIDELINES.md + src/ui/panels/README.md
7. 代码优先：IPathTracker / PurePursuitTracker / StanleyTracker / IPathPlanner / HybridAStarPlanner /
   BicycleModel / SimEngine / PriorityPathCoordinator / TimeWindowCollisionAvoidance /
   PlannerTrackerDialog / MonitorBridge / MonitorPanel / 各层 CMakeLists / tests/CMakeLists.txt

【Phase 6 完成定义（全部达成才可结束 Goal）】
A. MpcLateralTracker : IPathTracker — 沿参考轨迹线性化自行车误差动力学；有限时域；输出 steering_angle；有单测；可与 PP/Stanley 切换
B. QP 求解：教学 MVP 允许「小 horizon + Eigen 稠密 QP（箱约束/无约束+惩罚）」；若引入 OSQP/osqp-eigen，必须 BUILD_INTERFACE + Config find_dependency（禁止破坏 EXPORT）
C. StGraphSpeedPlanner（或同名）：沿已有 Path 建 (s,t) 图；把他车轨迹映射为 ST 障碍；输出速度剖面 / 时间戳路径；禁止只改「距离<d 停车」冒充 ST
D. SimEngine：tracker=mpc 分支；可选 speed_planner=st_graph|none；与 Priority/TimeWindow 协同或 ADR 写清边界
E. scenario / PlannerTrackerDialog 可切换 tracker: pure_pursuit|stanley|mpc；可开/关 ST 速度规划；bicycle 默认可保留 hybrid+stanley，MPC 为可选实验
F. UI：扩展 PlannerTrackerDialog 或新建 MpcStGraphDialog（禁止堆 ControlPanel/MainWindow）；Monitor 增加 MPC 预测误差或 ST 速度曲线至少一类
G. CMake：新源登记；保留 target_include_directories / target_link_libraries；Domain 零 Qt、零 rclcpp
H. 文档：ADR-014（线性 MPC）、ADR-015（ST-Graph 速度规划）；DEVELOPMENT_PLAN Phase6✅；SESSION_LOG；MUTATION M31+
I. 测试：MpcLateralTrackerTest、StGraphSpeedPlannerTest、MpcVsStanleyCompareTest 或扩展 PlannerSwitch*；用户本地或 ASCII 外置 Build + FleetSimTests 全绿才可 complete
J. 每会话 commit + push；四角色子 Agent 互相监督 PASS；回复四段式

【架构硬约束】
UI → App → Domain → Core
Domain/Core 禁止 #include <Qt*> 与 rclcpp
App 层一律 domain:: 前缀命名空间
接口先于实现：先 .h 后 .cpp
新 UI = 新/扩展 Dialog + CMake；MainWindow 仅挂载 ≤15 行
禁止编辑 .cursor/plans/
中文路径：ProjectManager 测试用仓库内 ASCII test_tmp；交付用户 Build 步骤

【四角色子 Agent 团队 — 每会话强制（主 Agent 编排，禁止自评 PASS）】
1. Planner（计划）：输出 mini-plan —— 允许改动 / NOT DO / 文件清单 / 测试清单 / 验收标准
2. Executor（执行）：只按 mini-plan 写代码与 CMake；禁止扩 scope
3. Tester（测试）：写/跑/补 GTest；缺测 → FAIL；假 MPC（=Stanley 换皮）→ FAIL；假 ST（=距离停车）→ FAIL
4. Reviewer（检查）：对照 §10 禁止偷懒 + Phase2–5 防回归 + UI_GUIDELINES；输出 PASS/FAIL + 问题列表
流程：Planner → Executor → Tester → Reviewer；任一 FAIL → 修复后从 Tester 或 Reviewer 重审，不得宣称会话完成。
可用 Task 工具拉 explore/generalPurpose 扮演四角色；主 Agent 只整合，不代替 Reviewer 盖章。

【建议 7 会话，可连续但禁止跳验收】
0: ADR-014/015 草案 + 接口调研 + 红灯测骨架（Mpc*/StGraph*）
1: MpcLateralTracker（线性误差模型 + 小 horizon QP）+ MpcLateralTrackerTest
2: SimEngine/scenario/Dialog 接入 tracker=mpc；与 Stanley 对比测
3: StGraphSpeedPlanner MVP（他车路径→ST 障碍 + 速度剖面）+ 单测
4: 与 Priority/TimeWindow 协同接线 + 多车回归
5: UI/Monitor（MPC/ST 指标）+ 场景资产
6: 端到端 + MUTATION M31+ + 四角色终审 + push + Phase6 ✅

【禁止偷懒】
1. 禁止 MPC = Stanley/PP 换皮（必须有预测时域 + 优化/QP 目标，即使是稠密小 QP）
2. 禁止 ST-Graph = 「距离 < d 就停车」
3. 禁止 Domain 写 Qt / rclcpp
4. 禁止把 MPC/ST 控件堆进 ControlPanel/MainWindow
5. 禁止新 Domain 类无 GTest；禁止跳过 MPC vs Stanley 对比测
6. 禁止削 CMake target_include_directories / target_link_libraries
7. 禁止 EXPORT 直接 PUBLIC 链未导出的 FetchContent（OSQP 等同 nlohmann 教训）
8. 禁止完整 Autoware / 完整 nav2 / 完整非线性大规模 MPC / 感知相机激光仿真
9. 禁止破坏 Phase3 TimeWindow、Phase5 Hybrid/Stanley/Priority 回归（除非 ADR 明确迁移+测）
10. 禁止 SESSION_LOG 缺「没做什么」；禁止跳过四角色监督
11. 禁止未更新 CMakeLists 就加源文件
12. 禁止测试写中文用户 Temp 导致 MinGW ofstream 假失败
13. 禁止 horizon>30 却无超时/失败路径（教学规模 N≤15～20）
14. 禁止把 Eigen 已有依赖删掉另起炉灶无 Qt 矩阵库

【Todos】
按会话 0→6 创建/更新；全部完成且 I/J 有测试绿证才 UpdateGoal complete。

从会话 0 开始立刻执行。中间不要换模型。
```

---

## §1 Phase 6 目标与验收标准

### 1.1 为何是 Phase 6（与学习路径对齐）

| 已完成（Phase 5） | Phase 6 要补的缺口 | 自动驾驶对应 |
|------------------|-------------------|--------------|
| Stanley / Pure Pursuit 几何跟踪 | **线性 MPC 横向跟踪**（预测 + 约束优化） | Autoware MPC Lateral / Apollo 控制层 |
| 几何路径 + Priority 占走廊 | **ST-Graph 速度规划**（沿参考线处理动态占位） | Apollo/工业 ST 速度规划 |
| 可切换 planner/tracker UI | 扩展 **mpc / st_graph** 实验开关 | 算法对比实验台 |

`docs/learning-path.md` 将 MPC、ST-Graph 标为 Phase 5 延伸 → **本阶段正式落地教学 MVP**。

### 1.2 验收清单（全部勾选 = Phase 6 完成）

- [ ] `MpcLateralTracker : IPathTracker`：误差状态、预测时域、QP/等价优化、输出 `steering_angle`；单测含收敛、限舵、低速
- [ ] `tracker: "mpc"` 可切换；DiffDrive/PP、bicycle/Stanley、Hybrid 回归不破
- [ ] `StGraphSpeedPlanner`：`(s,t)` 栅格或采样；他车轨迹投影为障碍；输出速度剖面；单测证明「绕开/减速」而非距离停车
- [ ] SimEngine 接线 + scenario 字段；与 TimeWindow/Priority **协同或 ADR-015 边界清晰**
- [ ] UI 独立扩展；Monitor 至少一条新曲线（MPC 横偏预测残差 **或** ST 速度剖面）
- [ ] ADR-014、ADR-015；DEVELOPMENT_PLAN Phase 6 ✅；SESSION_LOG；MUTATION M31+
- [ ] GTest +（用户本地 **或** ASCII 外置）FleetSimTests 全绿

### 1.3 明确不做（Phase 6 范围外）

| 项目 | 原因 |
|------|------|
| 完整 Autoware / Apollo 栈 | 依赖与部署爆炸 |
| 非线性 MPC + 完整动力学（侧偏/轮胎） | 调参与数值面过大；仍用运动学自行车 |
| 完整感知（激光/相机/预测网络） | 另阶段；他车用**已知仿真轨迹**填 ST |
| 最优大规模 CBS / EECBS | Phase5 Priority 已够教学 MAPF |
| 联合横纵向大系统 MPC（城市级） | 可 ADR 草稿，不阻塞 MVP |
| Domain 引入 Qt | 违反 ADR-001 |

---

## §2 当前框架结构（必须保持）

```
UI (Qt) → App (SimController / ProjectManager / FleetUiCoordinator / MonitorBridge)
        → Domain (SimEngine + planning/control/scheduling/collision/vehicle)
        → Core (Pose / Path / ControlCommand / EventBus / SimClock)

CMake:
FleetSimCore → FleetSimDomain → FleetSimApp → FleetSimUI → FleetSim
FleetSimTests → App + Domain + Core + GTest
已有：Eigen（FetchContent）、nlohmann（BUILD_INTERFACE）、GTest
bridges/ros2 可选 OFF
```

**Phase 6 增量（Domain 为主）：**

```
control/MpcLateralTracker.*          (+ 可选 DenseQpSolver 小工具)
planning/StGraphSpeedPlanner.*       (+ 可选 StGraph.* 数据结构)
扩展 SimEngine tick：tracker=mpc；可选先 ST 再跟踪
扩展 scenario：tracker / speed_planner 字段
扩展 UI：PlannerTrackerDialog 或新 Dialog
Monitor：新曲线数据源（App MonitorBridge）
```

**控制数据流（示意）：**

```
Path (Hybrid/A*) 
  → [可选] StGraphSpeedPlanner → 带速度/时间的参考
  → Tracker (PP | Stanley | MPC) → ControlCommand(δ,v)
  → Bicycle/DiffDrive integrate
并行：Priority 占走廊 + TimeWindow 速度缩放（勿无故删除）
```

---

## §3 优先阅读的程序（按优先级）

### P0 — 动手前必读

| 文件 | 为什么 |
|------|--------|
| `src/domain/control/IPathTracker.h` + `StanleyTracker.*` + `PurePursuitTracker.*` | MPC 必须并列实现同一接口 |
| `src/domain/vehicle/BicycleModel.*` + ADR-009 | 线性化模型与 `steering_angle` 语义 |
| `src/domain/SimEngine.cpp`（plan/tick/tracker 分支） | 注入点；Phase5 已有 `resolvedTrackerKind` |
| `src/domain/collision/PriorityPathCoordinator.*` + `TimeWindowCollisionAvoidance.*` | ST 不得替换为距离停车；须协同 |
| `src/ui/dialogs/PlannerTrackerDialog.*` | 扩展切换项，禁止新开堆进 Control |
| `src/app/MonitorBridge.*` + `MonitorPanel.*` | 横偏/航向已有；加 MPC/ST 指标 |
| Domain / tests `CMakeLists.txt` | 登记 + EXPORT 纪律 |

### P1 — 测试与资产锚点

| 文件 | 为什么 |
|------|--------|
| `tests/domain/StanleyTrackerTest.cpp` | MPC 对照风格 |
| `tests/domain/PlannerSwitchIntegrationTest.cpp` | 扩展 mpc 切换 |
| `tests/domain/PriorityCoordinatorTest.cpp` / `MultiAgvScenarioTest.cpp` | 多车回归 |
| `assets/scenarios/hybrid_narrow_turn/` / `bicycle_demo/` | MPC 弯道对比 |
| `tools/verify_phase5_evidence.py` | 仿写 `verify_phase6_evidence.py` |

### P2 — 防回归课（继续生效）

| ID | 对策 |
|----|------|
| P4-EXPORT | 新依赖（若 OSQP）只用 BUILD_INTERFACE + Config |
| P4-TEMP | 测试 ASCII `test_tmp` |
| P4-OPEN | Open Project 不覆盖 scenario model |
| P5-假实现 | MPC≠Stanley；ST≠距离停车 |
| P3-CMake | 禁止削 `target_*` |

---

## §4 网络调研摘要（执行 Agent 必读）

### 4.1 线性 MPC 路径跟踪（横向）

**共识（Autoware MPC Lateral / 教学实现）：**

1. 将路径跟踪化为**误差调节**：状态常用横偏 \(e\)、航向误差 \(\theta_e\)（及可选舵角 \(\delta\)）。  
2. 自行车运动学沿参考轨迹**线性化** → 时变/冻结线性模型 \(x_{k+1}=A x_k + B u_k\)。  
3. 有限时域 \(N\) 上最小化  
   \(\sum (e^\top Q e + u^\top R u) +\) 终端项；约束 \(\delta_{\min}\le u \le \delta_{\max}\)。  
4. 写成 **QP**；每控制周期解一次，只施加 \(u_0^*\)（receding horizon）。  
5. 调参：先固定 \(Q_{lat}\)，增大 \(R\) 抑振荡；\(N\) 与采样 \(\Delta t\) 权衡精度与算力。

**Phase 6 MVP 建议：**

| 项 | 选择 |
|----|------|
| 模型 | 运动学自行车误差模型（与 ADR-009 一致） |
| Horizon | \(N=8\sim15\)，\(\Delta t\approx0.05\sim0.1\) |
| 求解器 | **优先**：Eigen 稠密 QP（箱约束投影 / 惩罚法），零新依赖；**可选**：OSQP + osqp-eigen（须守 EXPORT） |
| 接口 | `MpcLateralTracker : IPathTracker` |
| 对照 | 同路径同场景 vs Stanley：弯道横偏或舵角变化可量化 |

**参考：**

- Autoware MPC Lateral：https://autowarefoundation.github.io/autoware_universe/main/control/autoware_mpc_lateral_controller/  
- Autoware MPC Algorithm：https://autowarefoundation.github.io/autoware.universe_planning/main/control/mpc_lateral_controller/model_predictive_control_algorithm/  
- osqp-eigen MPC 示例：https://robotology.github.io/osqp-eigen/md_pages_mpc.html  
- 开源自行车 MPC（Eigen+OSQP）：https://github.com/vTechSavyy/vehicle-mpc-controller  

### 4.2 ST-Graph 速度规划

**核心思想（分层规划主流）：**

1. 先有空间路径（FleetSim 已有 A*/Hybrid）。  
2. 建 **s–t 图**：纵轴沿路径弧长 \(s\)，横轴时间 \(t\)。  
3. 将他车预测/已知轨迹与自车路径求交，填成 ST 占用块。  
4. 在可行 ST 走廊内规划速度剖面（搜索、QP、或分段恒加速），满足 \(v,a\) 限幅。  
5. 输出带时间/速度的参考，供跟踪器执行。

**与 FleetSim 已有模块关系：**

| 模块 | 职责 | Phase6 |
|------|------|--------|
| Priority | 谁先占几何走廊 | 保留 |
| TimeWindow + Reservation | 运行时速度缩放 | 保留；ST 为**规划期**速度剖面 |
| ST-Graph | 沿参考线显式速度规划 | **新增** |

**禁止**：用欧氏距离阈值停车冒充 ST。

**参考：**

- CMU s–T / temporal optimization（IV）：https://www.cs.cmu.edu/~cliu6/files/iv17-2.pdf  
- MPQP + ST graph（arXiv 2401.06305）：https://arxiv.org/abs/2401.06305  
- Hybrid motion planning survey：https://arxiv.org/html/2406.05575v1  
- Apollo/工业实践：路径 + ST 速度是常见分解（本阶段只做教学规模）  

### 4.3 平台对照（了解边界）

Autoware：横向 PP **或** MPC，纵向常 PID。Apollo：横向 LQR / 混合 MPC 等。FleetSim Phase6 **只做教学线性 MPC + ST 速度 MVP**，不对标完整栈（参见 arXiv 2501.18942 平台对比）。

---

## §5 设计决策建议（先写 ADR 再写码）

### ADR-014 — 线性 MPC 横向跟踪

1. `MpcLateralTracker : IPathTracker`  
2. 构造参数：`horizon`、`dt`、`q_lat`、`q_heading`、`r_steer`、`max_steer`、`wheelbase`  
3. 输出：`steering_angle` + `linear_velocity`（速度可来自 ST 剖面或 cruise）  
4. 失败：QP 失败 → 回退限舵 Stanley 项 **或** 返回零舵并记日志（ADR 写死一种并测）

### ADR-015 — ST-Graph 速度规划

1. 输入：自车 `Path`、他车 `Path`+名义速度（或已预约表）、\(v_{\max},a_{\max}\)  
2. 输出：每 waypoint 的 \(v\) 或到达时间；SimEngine 跟踪前应用  
3. 与 TimeWindow：ST 定规划速度；TimeWindow 仍可在冲突时缩放（叠加）  
4. CBS-lite：不做；Priority 仍负责几何顺序  

---

## §6 建议新增/变更文件清单

### Domain

| 文件 | 职责 |
|------|------|
| `control/MpcLateralTracker.h/.cpp` | 线性 MPC |
| `control/DenseQpSolver.h/.cpp`（可选） | 小规模稠密 QP，隔离求解器 |
| `planning/StGraph.h/.cpp` | (s,t) 栅格与障碍填充 |
| `planning/StGraphSpeedPlanner.h/.cpp` | 速度剖面 |
| 扩展 `SimEngine` | tracker/speed_planner 分支 |
| 扩展 `ScenarioLoader` / Serializer | 字段 |

### UI / App

| 文件 | 职责 |
|------|------|
| 扩展 `PlannerTrackerDialog` **或** `dialogs/MpcStGraphDialog.*` | 切换 mpc / st_graph |
| `MonitorBridge` / `MonitorPanel` | 新曲线 |
| `panels/README.md` | 登记 |

### 测试 / 文档 / 工具

```
tests/domain/MpcLateralTrackerTest.cpp
tests/domain/StGraphSpeedPlannerTest.cpp
tests/domain/MpcVsStanleyCompareTest.cpp   # 或并入 PlannerSwitch*
docs/decisions/014-linear-mpc.md
docs/decisions/015-st-graph-speed.md
tools/verify_phase6_evidence.py
tools/run_phase6_verify.ps1
```

---

## §7 如何继续开发（Goal 节奏 + 四角色团队）

1. `git pull`；确认 Phase5：Hybrid/Stanley/Priority/Dialog 在；可选跑 `pwsh -File tools/run_phase5_verify.ps1`  
2. 会话 0：只 ADR + 红灯测  
3. 会话 1–2：MPC 绿 → 引擎/Dialog 切换 → 对比 Stanley  
4. 会话 3–4：ST-Graph 绿 → 多车协同 + 回归  
5. 会话 5–6：UI 指标 + MUTATION + **四角色终审** + push + Phase6 ✅  
6. 每会话：SESSION_LOG 四段 + 用户验证清单  

### 四角色监督协议（本阶段强制，取代「仅双 Reviewer」口头约定）

| 角色 | 输入 | 输出 | 否决权 |
|------|------|------|--------|
| **Planner** | Phase6 目标 + 本会话 scope | mini-plan：允许/禁止、文件表、测例名、NOT DO | 范围蔓延 |
| **Executor** | mini-plan | 代码 + CMake | — |
| **Tester** | 代码 + 测例清单 | 补测/红绿结论；假实现判定 | 无测/假实现 → FAIL |
| **Reviewer** | diff + §10 + UI 规范 | PASS/FAIL + 问题 | FAIL 阻会话完成 |

**规则：**

- 主 Agent **禁止**自评 Reviewer PASS。  
- Tester 与 Reviewer 必须是**独立** Task 子 Agent（或两次不同调用），不可同一回复里「自己测自己过」。  
- FAIL → 修复 → 至少再跑 Tester + Reviewer。  
- Scribe（可由主 Agent 兼任）写 SESSION_LOG，须引用四角色结论。

---

## §8 Phase 2–5 防回归（继续生效）

| ID | 对策 |
|----|------|
| P2-01 | App 一律 `domain::` |
| P2-02 | 先 .h 后 .cpp |
| P3-CMake | 改完 diff `target_*` |
| P4-EXPORT | FetchContent 勿直接 EXPORT |
| P4-TEMP | `test_tmp` ASCII |
| P4-OPEN | 勿默认 Settings 盖 scenario model |
| P5-Hybrid/Stanley | 回归测必须绿 |
| P3-避碰 | 禁止距离判碰替换 TimeWindow（无 ADR+测） |

---

## §9 关键参数默认值

| 参数 | 默认 | 说明 |
|------|------|------|
| MPC `horizon` | 10 | 教学规模 |
| MPC `dt` | 0.05～0.1 s | 与仿真 dt 同量级 |
| MPC `q_lat` | 1.0～5.0 | 横偏权重 |
| MPC `q_heading` | 1.0～5.0 | 航向权重 |
| MPC `r_steer` | 0.1～1.0 | 舵角惩罚 |
| ST 时间分辨率 | 0.1～0.2 s | 栅格 \(\Delta t\) |
| ST \(v_{\max}\) | 0.5 m/s | 与现车同量级 |
| ST \(a_{\max}\) | 0.5～1.0 m/s² | 教学 |
| tracker 默认 | 保持 Phase5：auto→PP；bicycle 场景可 stanley；**mpc 需显式选** | 防默默改回归 |

---

## §10 禁止偷懒清单（Reviewer 打印打勾）

1. 禁止 MPC = Stanley/PP 换皮  
2. 禁止 ST-Graph = 距离停车  
3. 禁止 Domain 含 Qt / rclcpp  
4. 禁止切换 UI 塞进 ControlPanel / MainWindow  
5. 禁止新 Domain 类无 GTest  
6. 禁止削 CMake `target_include_directories` / `target_link_libraries`  
7. 禁止 EXPORT 未导出的 FetchContent 目标  
8. 禁止完整 Autoware / nav2 / 大非线性 MPC / 感知仿真  
9. 禁止破坏 TimeWindow / Hybrid / Stanley / Priority 无 ADR  
10. 禁止 SESSION_LOG 缺「没做什么」  
11. 禁止不加 CMakeLists 就加源文件  
12. 禁止跳过四角色监督  
13. 禁止 horizon 过大无失败/超时策略  
14. 禁止中文 Temp 单测假红  

---

## §11 测试策略（最低标准）

| 层级 | 要求 |
|------|------|
| 单元 MPC | 直道横偏下降；限舵；QP/求解失败路径；低速不 NaN |
| 对比 | 同弯道：MPC 与 Stanley 横偏曲线可比较（不必处处更优，但行为可区分） |
| 单元 ST | 静态障碍 ST 占位；他车横穿 → 减速/等待剖面；空障碍退化为匀速 |
| 回归 | DiffDrive、MultiAgv、Hybrid*、Stanley*、Priority*、PlannerSwitch*、domain_smoke |
| 集成 | load→plan→(ST)→mpc tick→弯道姿态；Dialog 切换 tracker |
| 突变 | M31 MPC 预测矩阵符号；M32 ST 忽略他车占位；M33 QP 权重全 0 |

---

## §12 用户本地验证清单（每次交付必须附上）

1. `git pull origin main`  
2. （可选）`pwsh -File tools/run_phase6_verify.ps1`  
3. Qt Creator：**重新 Configure** → Build；**或** ASCII 外置：`D:\build\FleetSim_phase6`  
4. 运行 `FleetSimTests`（关注 Mpc* / StGraph* / Stanley* / Hybrid* / Priority* / MultiAgv*）  
5. Open `assets/scenarios/hybrid_narrow_turn` 或新建 `mpc_st_demo`  
6. File → Planner / Tracker：切 `stanley` / `mpc`，观察 Monitor  
7. 多车：确认 ST/Priority/TimeWindow 无「穿模静默」  
8. （可选）`fleet_domain_smoke`  

---

## §13 风险与缓解

| 风险 | 缓解 |
|------|------|
| QP 依赖把 EXPORT 再次弄挂 | 先 Eigen 稠密；OSQP 仅 BUILD_INTERFACE |
| MPC 振荡 | 增大 R；减小 N 步长；限舵 |
| ST 与 TimeWindow 双重减速过猛 | ADR 规定优先级；单测锁最小速度地板 |
| 算力 | N≤15；ST 栅格限制时间窗 |
| 假实现蒙混 | Tester 强制对比测 + 公式/矩阵断言 |

---

## §14 参考链接

1. https://autowarefoundation.github.io/autoware_universe/main/control/autoware_mpc_lateral_controller/  
2. https://autowarefoundation.github.io/autoware.universe_planning/main/control/mpc_lateral_controller/model_predictive_control_algorithm/  
3. https://robotology.github.io/osqp-eigen/md_pages_mpc.html  
4. https://github.com/vTechSavyy/vehicle-mpc-controller  
5. https://www.cs.cmu.edu/~cliu6/files/iv17-2.pdf  
6. https://arxiv.org/abs/2401.06305  
7. https://arxiv.org/html/2406.05575v1  
8. https://arxiv.org/abs/2501.18942（Autoware vs Apollo 对照，知边界）  
9. https://osqp.org/  

---

## §15 新对话开场白（短版，可与 §0 连用）

> 执行 `docs/PHASE6_GOAL_PROMPT.md`。Goal：完成 Phase 6（线性 MPC 横向跟踪 + ST-Graph 速度规划 + 可切换 UI）。强制四角色子 Agent（Planner / Executor / Tester / Reviewer）互相监督；足够 GTest；每会话 push；不改 plan 文件；中文路径 Build 交付验证步骤。从会话 0 开始，todos 标进度，直到验收清单全勾选且测试绿证齐全再 UpdateGoal complete。

---

## §16 Architect mini-plan 模板（Planner 角色每会话必填）

```markdown
**Phase**: 6
**本会话目标**: （一句话）
**允许改动**: （目录/文件）
**NOT DO**: （至少 5 条，含禁止假 MPC / 假 ST / 堆 UI / 削 CMake / 跨 Phase）
**预计新增/修改文件**:
**计划测试**（精确到 TEST 名）:
**验收标准**:
```
