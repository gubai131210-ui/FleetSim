# Phase 7 Goal 模式执行提示词（完整版）

> **用途**：新开对话后，整段复制「§0 一键粘贴 Prompt」给 Agent（Goal 模式）。  
> **仓库**：https://github.com/gubai131210-ui/FleetSim.git  
> **本地路径**：`d:\QT_CppPrograms\FleetSim\FleetSim`  
> **前置**：Phase 0–6 ✅（线性 MPC + ST-Graph + SimEngine 接线 + Monitor ST 曲线；FleetSimTests **104/104** 绿；`verify_phase6_evidence.py` 31 PASS）。  
> **本阶段主题**：**算法实验工作台（UI 信息架构重构）** + **教学向轨迹预测（Constant-Velocity）增强 ST** + **可导出对比指标**；与 Phase 5–6 的 Hybrid/Stanley/MPC/ST/Priority **并存、可对比、不堆控件**。  
> **禁止**：编辑 `.cursor/plans/`；完整 Autoware / 完整 nav2 / 神经网络预测 / 激光相机感知 / 完整 Behavior Tree 引擎；Agent 不代跑中文用户 Temp 破坏性操作；中文路径 Qt Build 由用户验证（Agent 可在 **ASCII** 路径如 `D:\build\FleetSim_*` 外置构建取证，交付仍须附「用户必做步骤」）。

---

## §0 一键粘贴 Prompt（Goal 模式直接用）

```
你是 FleetSim Phase 7 的执行 Agent（Goal 模式）。
仓库：d:\QT_CppPrograms\FleetSim\FleetSim
远程：https://github.com/gubai131210-ui/FleetSim.git
权威说明：docs/PHASE7_GOAL_PROMPT.md（必须先完整阅读再写代码）

【强制阅读顺序 — 未读完禁止写代码】
1. docs/PHASE7_GOAL_PROMPT.md（全文，尤其 §1–§12）
2. docs/DEVELOPMENT_PLAN.md、docs/learning-path.md（确认 Phase 6 ✅、Phase 7 目标）
3. docs/AGENT_SESSION_TEMPLATE.md
4. SESSION_LOG.md（Phase 6 Goal J / Session 6 条目；防回归：MPC/ST 假实现、EXPORT、中文 Temp、Phase3–6 回归）
5. docs/decisions/014-linear-mpc.md、015-st-graph-speed.md、013-priority-or-cbs-lite.md、001-architecture.md
6. docs/UI_GUIDELINES.md + src/ui/panels/README.md + docs/MUTATION_CHECKLIST.md
7. 代码优先：StGraphSpeedPlanner / SimEngine::collectPeersFor / refreshSpeedProfiles /
   MpcLateralTracker / PlannerTrackerDialog / MonitorBridge / MonitorPanel /
   Experiment 相关（Phase7 新建）/ 各层 CMakeLists / tests/CMakeLists.txt

【Phase 7 完成定义（全部达成才可结束 Goal）】
A. ConstantVelocityPredictor（或 IPeerPredictor 教学 MVP）：由 peer 当前 Pose + 名义速度外推有限时域轨迹；输出可供 ST 投影的 Path 或 spacetime 采样；有单测；禁止神经网络/感知
B. StGraphSpeedPlanner 增强：支持「静态 Path」与「预测轨迹」两种 peer 输入；SimEngine collectPeers 可配置 prediction=none|constant_velocity；单测证明预测改变 ST 剖面（非静态 Path 换皮）
C. ExperimentMetrics（Domain 或 App，零 Qt 若在 Domain）：记录 tick 级 cross-track、heading error、linear v、ST ref v、MPC lastSolveOk 等；可聚合为 RunSummary；有单测
D. UI 信息架构重构（硬合同）：
   - 新建 AlgorithmWorkbenchDialog（或等价命名）含 **分层页面**（Planning / Control / Speed / Coordination），禁止把四类配置堆在一个 Form 里
   - 新建 ExperimentComparePanel（独立 dock），展示对比指标/摘要；禁止堆进 ControlPanel/MainWindow/MonitorPanel 主曲线区
   - PlannerTrackerDialog 仅保留「打开工作台」或 deprecated 薄封装（≤15 行转发）；MainWindow 挂载 ≤15 行
   - MonitorPanel 保持轨迹误差+速度曲线；实验对比放 ExperimentComparePanel
E. scenario：assets/scenarios/prediction_st_demo（≥2 车，speed_planner=st_graph，可选 prediction）；assets/scenarios/experiment_compare_baseline（可选）
F. SimEngine/scenario：simulation.prediction 字段 none|constant_velocity；与 ST 接线；禁止 prediction 写了却不读
G. CMake 登记；Domain 零 Qt、零 rclcpp；保留 target_include_directories / target_link_libraries
H. 文档：ADR-016（轨迹预测 + ST）、ADR-017（UI 实验工作台 IA）；DEVELOPMENT_PLAN Phase7✅；SESSION_LOG；MUTATION M34+
I. 测试：ConstantVelocityPredictorTest、StGraphWithPredictionTest、ExperimentMetricsTest、ExperimentCompareIntegrationTest（或扩展 StGraphSimEngineWiring*）；至少 1 测：关 prediction 与开 prediction 的 ST 剖面须可区分；ASCII Build + FleetSimTests 全绿才可 complete
J. 每会话 commit + push；四角色互相监督 PASS；回复四段式

【架构硬约束】
UI → App → Domain → Core
Domain/Core 禁止 #include <Qt*> 与 rclcpp
App 层一律 domain:: 前缀命名空间
接口先于实现：先 .h 后 .cpp
新 UI = 新 Panel/Dialog + 分层 Page Widget；MainWindow 仅挂载
禁止编辑 .cursor/plans/
中文路径：ProjectManager 测试用仓库内 ASCII test_tmp

【四角色子 Agent 团队 — 每会话强制】
1. Planner（计划）：mini-plan — 允许改动 / NOT DO / 文件清单 / 测试清单 / 验收标准
2. Executor（执行）：只按 mini-plan 写代码与 CMake；禁止扩 scope
3. Tester（测试）：写/跑/补 GTest（优先 ASCII D:\build\FleetSim_*）；假预测（=静态 Path 换名）→ FAIL；UI 堆控件 → FAIL
4. Reviewer（检查）：对照 §10 + UI_GUIDELINES + Phase2–6 防回归；输出 PASS/FAIL
流程：Planner → Executor → Tester → Reviewer。主 Agent 禁止自评 PASS。SESSION_LOG 须引用四角色结论。

【建议 8 会话，可连续但禁止跳验收】
0: ADR-016/017 草案 + 接口调研 + 红灯测骨架（Predictor*/ExperimentMetrics*）
1: ConstantVelocityPredictor + 单测转绿
2: StGraph + SimEngine prediction 接线 + StGraphWithPredictionTest
3: ExperimentMetrics（Domain）+ ExperimentMetricsTest
4: UI — AlgorithmWorkbenchDialog 分层 Page + scenario 字段
5: UI — ExperimentComparePanel + MonitorBridge 指标扩展 + 场景资产
6: 对比集成测 + Phase3–6 回归
7: MUTATION M34+ + verify_phase7 + 四角色终审 + push + Phase7 ✅

【禁止偷懒】
1. 禁止 prediction = 静态 peer Path 换名（须有外推公式 + 时域 + 单测断言）
2. 禁止 UI 把 Planning/Control/Speed/Coordination/对比曲线堆进同一页或 PlannerTrackerDialog 加长
3. 禁止 ExperimentComparePanel 内容塞进 MonitorPanel 底部或 ControlPanel
4. 禁止 Domain 写 Qt / rclcpp
5. 禁止新 Domain 类无 GTest
6. 禁止削 CMake target_*
7. 禁止破坏 Phase6 MPC/ST/TimeWindow/Priority 回归
8. 禁止完整 Autoware/nav2/BehaviorTree/感知仿真
9. 禁止 SESSION_LOG 缺四角色 / 缺「没做什么」
10. 禁止未更新 CMakeLists 就加源文件
11. 禁止「写 ADR 说不做」规避 prediction 读 peer 状态
12. 禁止 Phase7 未做 UI 分层却标 Phase7 ✅

从会话 0 开始立刻执行。中间不要换模型。
```

---

## §1 Phase 7 目标与验收标准

### 1.1 为何是 Phase 7（与学习路径对齐）

| 已完成（Phase 6） | Phase 7 要补的缺口 | 自动驾驶对应 |
|------------------|-------------------|--------------|
| MPC 横向 + ST 纵向（静态 peer Path） | **轨迹预测** 喂 ST（短 horizon CV） | Apollo EM / 工业 ST 动态障碍 |
| PlannerTrackerDialog 单页堆 planner/tracker/ST | **实验工作台 UI**（按层分页） | Nav2 插件/BT 分层思想（教学简化） |
| Monitor 单曲线 | **对比指标面板**（算法 A vs B） | 仿真验证 / 指标导出 |

Phase 6 已具备「规划 + 控制 + 速度 + 协调」全链路，但 **UI 与实验流程** 仍不适合系统化对比；ST 仍主要假设 **他车 Path 已知静态**，缺工业界第一步 **prediction → ST 投影** 的教学闭环。

### 1.2 验收清单（全部勾选 = Phase 7 完成）

- [ ] `ConstantVelocityPredictor`（或 `IPeerPredictor`）：外推 horizon、输出可投影轨迹；单测覆盖直线/转弯名义速度
- [ ] `StGraphSpeedPlanner` / SimEngine：`prediction=constant_velocity` 时 peer 障碍来自预测，非仅静态 Path；关/开 prediction 剖面可区分
- [ ] `ExperimentMetrics`：tick 采样 + RunSummary 聚合；单测
- [ ] **AlgorithmWorkbenchDialog**：≥4 个独立 Page Widget（Planning/Control/Speed/Coordination），每页单一职责
- [ ] **ExperimentComparePanel**：独立 dock；展示至少 2 类对比指标（如 mean |e|、min ST v、MPC solve rate）
- [ ] 未在 MainWindow/ControlPanel/MonitorPanel 堆新表单/曲线
- [ ] ADR-016、ADR-017；DEVELOPMENT_PLAN Phase 7 ✅；SESSION_LOG；MUTATION M34+
- [ ] GTest + ASCII 外置 Build `FleetSimTests` 全绿（≥Phase6 104 + 新增测）

### 1.3 明确不做（Phase 7 范围外）

| 项目 | 原因 |
|------|------|
| 完整 Autoware / Apollo / nav2 栈 | 依赖与部署爆炸 |
| 神经网络 / 多模态预测 | 另阶段；本阶段 CV 足够 |
| 激光/相机/占用栅格感知 | 另阶段 |
| 完整 Behavior Tree 引擎 | 可 ADR 草稿「未来」；不阻塞 MVP |
| LaneGraph 真路由 | 可 Phase 8；LaneGraph 仍 stub |
| EECBS 最优 MAPF | Phase5 Priority 已够教学 |
| 非线性 MPC / 联合大系统 | Phase6 ADR 已界定 |

---

## §2 当前框架结构（必须保持）

```
UI (Qt) → App (SimController / ProjectManager / MonitorBridge / ExperimentBridge*)
        → Domain (SimEngine + planning/control/scheduling/collision/prediction*)
        → Core (Pose / Path / SpeedProfile / EventBus / SimClock)

CMake:
FleetSimCore → FleetSimDomain → FleetSimApp → FleetSimUI → FleetSim
FleetSimTests → App + Domain + Core + GTest
已有：Eigen、nlohmann（BUILD_INTERFACE）、GTest
bridges/ros2 可选 OFF
```

**Phase 7 增量（示意）：**

```
domain/prediction/ConstantVelocityPredictor.*   (+ 可选 IPeerPredictor.h)
domain/experiment/ExperimentMetrics.*         (或 app/ExperimentRecorder.* 若需 Qt 则放 App)
planning/StGraphSpeedPlanner.*                (读预测轨迹)
ui/dialogs/AlgorithmWorkbenchDialog.*         (+ pages/PlanningPage.* ControlPage.* SpeedPage.* CoordinationPage.*)
ui/panels/ExperimentComparePanel.*
app/ExperimentBridge.*                        (可选：连接 Metrics → Panel)
扩展 SimEngine：prediction_kind_；collectPeers 用 predictor
扩展 scenario：simulation.prediction
```

**数据流（Prediction → ST，示意）：**

```
他车 Agent (Pose, v, reference_path)
  → [可选] ConstantVelocityPredictor → predicted Path / spacetime samples
  → StGraphSpeedPlanner(ego_path, peers') → SpeedProfile
  → Tracker (MPC/Stanley/PP) + TimeWindow scale
并行：Priority + Reservation（不变）
```

---

## §3 优先阅读的程序（按优先级）

### P0 — 动手前必读

| 文件 | 为什么 |
|------|--------|
| `src/domain/planning/StGraphSpeedPlanner.*` | Phase7 增强 prediction 输入 |
| `src/domain/SimEngine.cpp`（collectPeersFor / refreshSpeedProfiles / tick） | prediction 注入点 |
| `src/domain/control/MpcLateralTracker.*` | 对比指标 MPC solve |
| `src/app/MonitorBridge.*` | 扩展实验指标信号 |
| `src/ui/dialogs/PlannerTrackerDialog.*` | **待拆分**；Phase7 勿继续堆控件 |
| `src/ui/panels/MonitorPanel.*` | 保持轨迹监控；对比放新 Panel |
| `src/ui/MainWindow.cpp`（handlePlannerTracker / dock 挂载） | 新 Workbench/Compare 挂载 ≤15 行 |
| `tests/domain/StGraphSimEngineWiringTest.cpp` | Phase6 多车 ST 合同；Phase7 扩展 prediction |
| Domain/tests `CMakeLists.txt` | 登记 + EXPORT 纪律 |

### P1 — UI 与实验

| 文件 | 为什么 |
|------|--------|
| `docs/UI_GUIDELINES.md` | Reviewer 硬规则 |
| `src/ui/panels/README.md` | 登记 ExperimentComparePanel |
| `assets/scenarios/mpc_st_demo/` | Phase7 场景模板 |
| `tools/verify_phase6_evidence.py` | 仿写 verify_phase7 |

### P2 — 防回归（继续生效）

| ID | 对策 |
|----|------|
| P6-MPC/ST | M31–M33 仍有效；Phase7 不得破坏 |
| P4-EXPORT | 新依赖 BUILD_INTERFACE |
| P4-TEMP | test_tmp ASCII |
| P6-ST | WithPeersProfileDiffersFromEmptyPeers 仍绿 |
| P3-CMake | 禁止削 `target_*` |

---

## §4 网络调研摘要（执行 Agent 必读）

### 4.1 自动驾驶栈在 ST-Graph 之后的常见下一层

工业/教学栈在「几何路径 + ST 速度 + 跟踪控制」之后，通常补齐：

1. **Prediction（预测）**：对他车/行人未来轨迹做短 horizon 外推，再投影到 ST 图（Apollo EM Planner、Moving Target Interception 等均采用 **常速或分段常速** 作为短 horizon 假设）。  
2. **Behavior / Decision（行为决策）**：在 motion planning 前决定 yield / nudge / lane-change（Nav2 用 Behavior Tree 编排 planner/controller/recovery 插件）。  
3. **Metrics & Validation（指标与验证）**：仿真中记录轨迹误差、约束违反、对比不同 planner/controller 配置（Autoware/Apollo 生态依赖大量 bag/指标；FleetSim 用 **ExperimentMetrics + CSV** 教学替代）。

**Phase 7 选择**：只做 **(1) CV 预测增强 ST** + **(3) 实验指标与 UI 工作台**；**(2) 完整 BT** 明确不做，仅在 ADR-017 写「未来可接 Nav2 式插件编排」。

### 4.2 常速预测 + ST-Graph（教学 MVP 共识）

| 项 | 工业/论文共识 | FleetSim Phase7 |
|----|--------------|-----------------|
| 短 horizon | CV 足够（~3–5 s 内） | 默认 3 s，Δt 与 ST dt_grid 对齐 |
| 输入 | 位置、航向、速度 | peer `Vehicle::pose()` + nominal_speed |
| 输出 | 时空采样或折线 Path | `core::Path` 稠密采样或 `PredictedTrajectory` |
| ST 投影 | 障碍映射为 (s,t) 占用块 | 复用 StGraphSpeedPlanner 现有投影 |
| 禁止 | 静态障碍当动态 | 单测：开 prediction 与关 prediction 剖面不同 |

参考：Apollo EM Planner ST 图（动态障碍 CV 投影）；Moving Target Interception (arXiv:2205.07772)；Two-Stage Spatiotemporal Trajectory (Glasgow)。

### 4.3 UI：Nav2 / Autoware 对 FleetSim 的启示（边界内）

| 平台 | 启示 | FleetSim Phase7 落地 |
|------|------|---------------------|
| **Nav2** | Behavior Tree 编排独立 server；planner/controller 插件分离 | **Workbench 按 Planning/Control/Speed/Coordination 分页**，不实现 BT |
| **Autoware 2.0** | Generator–Selector：多轨迹候选 + 选择 | **ExperimentComparePanel** 对比多配置运行结果（非多轨迹并行） |
| **FleetSim** | 中央 MapView + Dock | 新 Panel/Dialog，禁止 centralWidget 堆表单 |

### 4.4 平台对照（知边界）

Autoware/Apollo 含完整感知预测网络；FleetSim Phase7 **只做 CV 预测 + 实验 UI**，不对标完整栈（参见 Trajectory Prediction survey arXiv:2503.03262）。

---

## §5 设计决策建议（先写 ADR 再写码）

### ADR-016 — 常速轨迹预测 + ST 增强

1. 接口（写死一种）：

```cpp
class ConstantVelocityPredictor {
public:
    core::Path predictPath(const core::Pose& current,
                           double nominal_speed_mps,
                           double horizon_s,
                           double sample_dt_s) const;
};
```

2. `PeerTrajectory` 扩展或并行结构：`predicted_path` + `use_prediction` 标志。  
3. SimEngine：`simulation.prediction`: `none` | `constant_velocity`（默认 `none` 保 Phase6 回归）。  
4. `collectPeersFor`：若 `constant_velocity`，用 predictor 生成 path 再填 ST；**禁止**仍读静态 path 却标 prediction 开。  
5. 单测：直线匀速外推长度 ≈ v·T；ST 剖面随 prediction 变化。

### ADR-017 — UI 算法实验工作台信息架构

1. **AlgorithmWorkbenchDialog** 结构（写死）：

```
AlgorithmWorkbenchDialog
├── PlanningPage      (planner: auto/astar/hybrid_astar)
├── ControlPage       (tracker: auto/pp/stanley/mpc)
├── SpeedPage         (speed_planner: none/st_graph; prediction: none/cv)
└── CoordinationPage  (coordination: priority/none)
```

2. **ExperimentComparePanel**（独立 dock）：显示当前 run 与 baseline 的 aggregated metrics（表格或迷你曲线）；**禁止**与 MonitorPanel 合并。  
3. **PlannerTrackerDialog**：deprecated 为「打开 AlgorithmWorkbench…」菜单入口，或删除表单仅留转发（Reviewer 检查 diff 行数）。  
4. MainWindow 新增 dock 挂载 ExperimentComparePanel ≤15 行。  
5. 每 Page 单独 `.h/.cpp`，单文件 ≤300 行；超过拆 SubWidget。

---

## §6 建议新增/变更文件清单

### Domain / Core

| 文件 | 职责 |
|------|------|
| `prediction/ConstantVelocityPredictor.h/.cpp` | CV 外推 |
| `prediction/IPeerPredictor.h`（可选） | 扩展点 |
| `experiment/ExperimentMetrics.h/.cpp` | tick 采样与聚合 |
| 扩展 `StGraphSpeedPlanner.*` | 预测轨迹输入 |
| 扩展 `SimEngine.*` | prediction_kind / collectPeers |

### UI / App

| 文件 | 职责 |
|------|------|
| `ui/dialogs/AlgorithmWorkbenchDialog.*` | 壳 + QStackedWidget |
| `ui/dialogs/pages/PlanningPage.*` 等 4 页 | 分层配置 |
| `ui/panels/ExperimentComparePanel.*` | 对比指标 |
| `app/ExperimentBridge.*`（可选） | Metrics → UI |
| 扩展 `MonitorBridge.*` | 可选 MPC solve 标志 |

### 测试 / 文档 / 工具

```
tests/domain/ConstantVelocityPredictorTest.cpp
tests/domain/StGraphWithPredictionTest.cpp
tests/domain/ExperimentMetricsTest.cpp
tests/integration/ExperimentCompareIntegrationTest.cpp
docs/decisions/016-constant-velocity-prediction.md
docs/decisions/017-algorithm-workbench-ui.md
tools/verify_phase7_evidence.py
tools/run_phase7_verify.ps1
assets/scenarios/prediction_st_demo/
```

---

## §7 如何继续开发（Goal 节奏 + 四角色）

1. `git pull`；确认 Phase6：`pwsh -File tools/run_phase6_verify.ps1`；FleetSimTests 104 绿  
2. 会话 0：ADR + 红灯测  
3. 会话 1–2：Predictor 绿 → ST/SimEngine 接线  
4. 会话 3：ExperimentMetrics  
5. 会话 4–5：UI 分层 + Compare Panel（**Tester 必须 FAIL 堆 UI 的 diff**）  
6. 会话 6–7：回归 + MUTATION + 终审 + push  

### 四角色监督协议

| 角色 | 输入 | 输出 | 否决权 |
|------|------|------|--------|
| **Planner** | Phase7 目标 + 本会话 scope | mini-plan | 范围蔓延 |
| **Executor** | mini-plan | 代码 + CMake | — |
| **Tester** | 代码 + 测例清单 | 红绿 + 假实现判定 | 无测/假 prediction/UI 堆叠 → FAIL |
| **Reviewer** | diff + §10 + UI_GUIDELINES | PASS/FAIL | FAIL 阻完成 |

**UI 专项 Tester 检查**：若 `PlannerTrackerDialog.cpp` 行数显著增加且未新建 Page 文件 → FAIL。

---

## §8 Phase 2–6 防回归（继续生效）

| ID | 对策 |
|----|------|
| P6-MPC/ST | Mpc*/StGraph*/Wiring 测仍绿 |
| P6-EXPORT | FetchContent 纪律 |
| P4-TEMP | test_tmp ASCII |
| P5-Hybrid/Stanley/Priority | 回归测绿 |
| P7-UI | 新功能必须在 Workbench/Compare 新文件 |

---

## §9 关键参数默认值

| 参数 | 默认 | 说明 |
|------|------|------|
| prediction horizon | 3.0 s | 教学 |
| prediction sample_dt | 0.1 s | 与 ST 对齐 |
| simulation.prediction | none | 显式开 cv |
| ExperimentMetrics 窗口 | 500 ticks | 与 Monitor 一致 |

---

## §10 禁止偷懒清单（Reviewer 打印打勾）

1. 禁止 prediction = 静态 Path 换名  
2. 禁止 ST 不读 prediction 状态  
3. 禁止 Planning/Control/Speed/Coordination 堆同一 Dialog 页  
4. 禁止 ExperimentCompare 堆 Monitor/Control/MainWindow  
5. 禁止 Domain 含 Qt / rclcpp  
6. 禁止新 Domain 类无 GTest  
7. 禁止削 CMake `target_*`  
8. 禁止完整 Autoware/nav2/感知/BT  
9. 禁止破坏 Phase6 MPC/ST/TimeWindow/Priority  
10. 禁止 SESSION_LOG 缺四角色 / 缺「没做什么」  
11. 禁止未登记 CMake 就加源  
12. 禁止跳过四角色  
13. 禁止 Phase7 ✅ 但 UI 仍单页 PlannerTrackerDialog 堆控件  

---

## §11 测试策略（最低标准）

| 层级 | 要求 |
|------|------|
| 单元 Predictor | 外推长度、点数、horizon 边界 |
| 单元 ST+Prediction | 开/关 prediction 剖面不同 |
| 单元 Metrics | 聚合 mean |e|、样本数 |
| 集成 | Workbench 设置写入 SimEngine；ComparePanel 收到非空 summary |
| 回归 | Phase6 全部 Mpc/StGraph/Wiring + Hybrid/Stanley/Priority/MultiAgv |
| 突变 | M34 预测 horizon=0；M35 忽略 CV 外推；M36 Metrics 不采样 |

---

## §12 用户本地验证清单（每次交付必须附上）

1. `git pull origin main`  
2. 可选：`pwsh -File tools/run_phase7_verify.ps1`  
3. Qt Creator 重新 Configure → Build；或 ASCII：`D:\build\FleetSim_phase7`  
4. 运行 `FleetSimTests`（关注 Predictor* / Experiment* / StGraph* / Mpc*）  
5. Open `assets/scenarios/prediction_st_demo`  
6. 菜单打开 **Algorithm Workbench** → 分 Tab 设置 Planning/Control/Speed/Coordination  
7. 查看 **Experiment Compare** dock；Monitor 仍显示误差/速度  
8. 对比 prediction none vs constant_velocity 下 ST ref v 曲线差异  

---

## §13 风险与缓解

| 风险 | 缓解 |
|------|------|
| UI  refactor 范围蔓延 | ADR-017 写死 4 页 + 1 Compare Panel |
| Agent 继续堆 PlannerTrackerDialog | Reviewer 行数阈值 + 必须新建 pages/ |
| Prediction 假实现 | 外推公式单测 + ST 对比测 |
| Phase6 回归破 | 默认 prediction=none |

---

## §14 参考链接

1. [Nav2 Architecture](https://docs.nav2.org/) — 插件化 planner/controller/BT 分层  
2. [Autoware 2.0 Generator-Selector](https://autowarefoundation.github.io/autoware-documentation/main/design/autoware-architecture-v2/)  
3. [Apollo EM Planner / ST Graph 教学说明](https://doi.org/10.48550/arxiv.2307.00482)  
4. [Moving Target Interception + ST Graph (arXiv:2205.07772)](https://doi.org/10.48550/arxiv.2205.07772)  
5. [Trajectory Prediction Survey (arXiv:2503.03262)](https://arxiv.org/html/2503.03262v1)  
6. Phase6：`docs/PHASE6_GOAL_PROMPT.md`、`docs/decisions/014-linear-mpc.md`、`015-st-graph-speed.md`

---

## §15 新对话开场白（短版）

> 执行 `docs/PHASE7_GOAL_PROMPT.md`。Goal：完成 Phase 7（CV 预测增强 ST + 算法实验工作台 UI 分层 + ExperimentComparePanel + 指标导出）。强制四角色子 Agent；GTest + ASCII Build 全绿；每会话 push。从会话 0 开始。

---

## §16 Planner mini-plan 模板（每会话必填）

```markdown
**Phase**: 7
**本会话目标**: （一句话）
**允许改动**: （目录/文件）
**NOT DO**: （≥5 条，含禁止堆 UI / 假 prediction / 破坏 Phase6）
**预计新增/修改文件**:
**计划测试**（精确 TEST 名）:
**UI 分页验收**（若涉及 UI：列出必须独立的 Page/Panel 文件名）:
**验收标准**:
```
