# Phase 4 Goal 模式执行提示词（完整版）

> **用途**：新开对话后，整段复制「§0 一键粘贴 Prompt」给 Agent（Goal 模式）。  
> **仓库**：https://github.com/gubai131210-ui/FleetSim.git  
> **本地路径**：`d:\QT_CppPrograms\FleetSim\FleetSim`  
> **前置**：Phase 0–3 已完成（多 AGV + Greedy + 时间窗口避碰）。Phase 4 = Bicycle 模型 + Domain 静态库导出 + ROS2 桥接（可选 Hungarian）。  
> **禁止**：编辑 `.cursor/plans/` 下的计划文件；Agent 不代跑中文路径下的 Qt Build（由用户本地验证）。

---

## §0 一键粘贴 Prompt（Goal 模式直接用）

```
你是 FleetSim Phase 4 的执行 Agent。仓库：d:\QT_CppPrograms\FleetSim\FleetSim
远程：https://github.com/gubai131210-ui/FleetSim.git

【强制阅读顺序 — 未读完禁止写代码】
1. docs/PHASE4_GOAL_PROMPT.md（本文全部章节，尤其 §1–§8、§10–§12）
2. docs/DEVELOPMENT_PLAN.md
3. docs/AGENT_SESSION_TEMPLATE.md
4. SESSION_LOG.md（最近 Phase 2/3 条目与防回归教训）
5. docs/decisions/001-architecture.md
6. docs/decisions/008-multi-vehicle-engine.md
7. docs/UI_GUIDELINES.md + src/ui/panels/README.md
8. 代码优先阅读清单见本文 §3

【Phase 4 目标（必须全部达成才算完成）】
A. Bicycle 运动学模型：可替换 DiffDrive；scenario 可选 model=bicycle；跟踪输出适配舵角
B. Domain 可脱离 Qt 导出静态库（FleetSimCore + FleetSimDomain），有 CMake export / 安装或 find 示例
C. ROS2 桥接：独立目录（建议 ros2_ws/ 或 bridges/ros2/），通过 EventBus/JSON 或薄适配层发布 pose/path/task；Domain 零 rclcpp、零 Qt
D. SettingsDialog：独立对话框切换车辆模型 / 关键参数；禁止堆进 ControlPanel
E. （推荐同阶段或紧随）HungarianAssigner 实现 ITaskAssigner，可配置切换 Greedy/Hungarian；有对比单测
F. 文档：ADR-009（Bicycle）、ADR-010（ROS2 桥）、更新 DEVELOPMENT_PLAN Phase 4 ✅、SESSION_LOG、MUTATION_CHECKLIST
G. 测试：所有新 Domain 类有 GTest；新增 BicycleModelTest、（HungarianAssignerTest）、BicycleScenarioTest 或扩展 MultiAgvScenarioTest；用户本地 Build + Run Tests 通过才可标 complete

【架构硬约束】
UI → App → Domain → Core
Domain / Core 禁止 #include <Qt*> 与 rclcpp
App 层命名空间一律写全：domain::map::、domain::scenario::、domain::scheduling::（禁止裸 map::）
接口先于实现：先改 .h 再写 .cpp；会话末 grep 对齐
新 UI = 新 Panel/Dialog 文件 + 更新 CMakeLists；MainWindow 只挂载 ≤15 行
每会话 commit + push origin main；中文路径不代跑编译，交付「用户必做 Build 检查项」

【子 Agent 互相监督（强制）】
每个工作会话必须按角色拆分（可用 Task 子 agent），禁止一人包办不审查：
1. Architect：写 mini-plan（允许改动 / NOT DO / 预计文件）
2. Implementer：只按 mini-plan 实现
3. Reviewer-Code：对照 §10 禁止偷懒清单 + Phase2/3 防回归；检查头/实现同步、命名空间、lambda capture、CMake include/link
4. Reviewer-Test：确认新类有单测、边界用例、集成场景；缺失则 FAIL 打回 Implementer
5. Reviewer-UI：对照 UI_GUIDELINES；SettingsDialog 是否独立
6. Scribe：按 AGENT_SESSION_TEMPLATE 更新 SESSION_LOG；回复用户四段式摘要
任一 Reviewer FAIL → 不得 push「完成」声明，必须修复后再审。

【会话拆分建议（6 会话，可按 Goal 连续执行但禁止跳验收）】
会话0：调研固化 + ControlCommand 扩展 ADR + Vehicle 模型抽象 stub + ADR-009 草案
会话1：BicycleModel 实装 + Vehicle 策略切换 + 单测
会话2：跟踪适配（PurePursuit→舵角 或 Stanley stub）+ scenario model 字段 + bicycle 场景资产
会话3：CMake 导出 FleetSimDomain 静态库 + 无 Qt 的 smoke 示例/文档
会话4：ROS2 桥包（可选编译）+ EventBus→topic 映射 + ADR-010
会话5：SettingsDialog +（Hungarian）+ 端到端 + 文档 + 双 Reviewer + push

【禁止偷懒 — Phase 4 专用】
1. 禁止把 Bicycle 写成「DiffDrive 改名」而不用舵角/轴距方程
2. 禁止在 Domain 内写 ROS2 或 Qt
3. 禁止把模型切换控件堆进 ControlPanel/MainWindow
4. 禁止跳过静态库导出，只口头说「可复用」
5. 禁止 ROS2 桥与主工程强耦合导致无 ROS 环境无法编主工程
6. 禁止新类无单测；禁止跳过 Bicycle 相关集成测
7. 禁止削掉 CMake target_include_directories / target_link_libraries（Phase3 曾踩坑）
8. 禁止跨阶段做 MPC / Autoware / 完整 nav2 栈
9. 禁止 SESSION_LOG 不写「没做什么」
10. 禁止未更新 CMakeLists 就加文件

【完成定义】
- DEVELOPMENT_PLAN Phase 4 标 ✅
- SESSION_LOG 有完整 Phase 4 条目
- GitHub main 已 push
- 回复含：做了什么 / 没做什么 / 为何拆分 / 用户本地验证步骤
```

---

## §1 Phase 4 目标与验收标准

### 1.1 官方计划（`docs/DEVELOPMENT_PLAN.md`）

| Phase | 目标 | 预估 |
|-------|------|------|
| **4** | Bicycle 模型 + Domain 静态库 + ROS2 桥接 | 3 周 |

学习路径对应（`docs/learning-path.md`）：乘用车级控制概念 + 中间件；Domain 保持无 Qt，导出静态库或 JSON 桥接 ROS2 topic。

### 1.2 验收清单（全部勾选 = Phase 4 完成）

- [ ] `BicycleModel`：后轴参考运动学积分；参数含 `wheelbase_m`、最大舵角
- [ ] `Vehicle`（或策略接口）可按 scenario `model: "diff_drive" | "bicycle"` 切换
- [ ] `ControlCommand` 扩展或并行结构支持舵角（见 §5 ADR 建议）；DiffDrive 路径不破坏
- [ ] 跟踪：Bicycle 车可用 Pure Pursuit 曲率→舵角，或提供 `StanleyTracker`（可选）
- [ ] `assets/scenarios/` 至少 1 个 bicycle 场景；Open → Play 可见差异行为
- [ ] CMake：`FleetSimCore`/`FleetSimDomain` 可 `install`/`export`；文档说明如何在无 Qt 目标中链接
- [ ] ROS2 桥：独立包/目录；`FLEETSIM_BUILD_ROS2=OFF` 默认；开启后发布至少 `pose`/`path`
- [ ] `SettingsDialog`：模型与关键参数；独立文件
- [ ] 单测 + 集成测 green（用户本地确认）
- [ ] ADR-009、ADR-010；SESSION_LOG；DEVELOPMENT_PLAN Phase 4 ✅

### 1.3 明确不做（Phase 4 范围外）

| 项目 | 原因 |
|------|------|
| 完整 Autoware / nav2 栈 | 过大，属后续 |
| 动力学自行车（侧偏角/轮胎力） | Phase 4 仅运动学 |
| MPC 跟踪 | Phase 5+ |
| 把 Qt UI 搬进 ROS | 违反分层 |
| 替换时间窗口避碰为简单距离 | 禁止回退 Phase 3 质量 |

---

## §2 当前框架结构（必须保持）

```
UI (Qt Widgets)  →  App (SimController / FleetUiCoordinator / ProjectManager)
                 →  Domain (SimEngine + Fleet/Planning/Control/Scheduling/Collision)
                 →  Core (Pose / Path / Task / EventBus / SimClock)

CMake:
FleetSimCore → FleetSimDomain → FleetSimApp → FleetSimUI → FleetSim(EXE)
FleetSimTests → FleetSimApp + Domain + Core + GTest
```

**Phase 4 增量：**

```
FleetSimDomain (STATIC, exportable, NO Qt, NO ROS)
       ↑
bridges/ros2 或 ros2_ws/fleetsim_bridge  (可选, rclcpp)
       ↑
Qt App 仍走 FleetSimApp（不变）
```

**SimEngine 多车 tick（Phase 3 已有，勿破坏）：**

```
scheduling.tick → replan → collision.reserve → collision.tick(speed_scale)
→ tracker.compute → model.integrate → EventBus(pose/path/task)
```

---

## §3 优先阅读的程序（按优先级）

### P0 — 动手前必读

| 文件 | 为什么 |
|------|--------|
| `src/domain/vehicle/Vehicle.h/.cpp` | 当前死绑 `DiffDriveModel`，Phase 4 改造中心 |
| `src/domain/vehicle/DiffDriveModel.*` | 对照实现 Bicycle |
| `src/core/types/ControlCommand.h` | 现仅 linear/angular，Bicycle 需舵角语义 |
| `src/domain/control/PurePursuitTracker.*` | 输出 angular；Bicycle 需曲率→δ |
| `src/domain/control/IPathTracker.h` | 策略接口 |
| `src/domain/SimEngine.cpp` | tick 里 integrate / publish |
| `src/domain/scenario/ScenarioLoader.*` / `ScenarioSerializer.*` | vehicles[].model 字段 |
| `src/app/SimController.*` | 加载场景与车辆 |
| `CMakeLists.txt` + `src/*/CMakeLists.txt` | **曾被截断导致编译失败**；改完必须保留 include/link |

### P1 — 调度与测试锚点

| 文件 | 为什么 |
|------|--------|
| `src/domain/scheduling/ITaskAssigner.h` / `GreedyAssigner.*` | Hungarian 扩展点 |
| `tests/integration/MultiAgvScenarioTest.cpp` | 回归锚；扩展 bicycle |
| `tests/domain/VehicleTest.cpp` / `PurePursuitTrackerTest.cpp` | 模式 |

### P2 — UI / 协调

| 文件 | 为什么 |
|------|--------|
| `src/ui/MainWindow.*` | 只挂 SettingsDialog |
| `src/app/FleetUiCoordinator.*` | 多车图元；勿再塞回 MainWindow |
| `src/ui/panels/ControlPanel.*` | **禁止**在此堆模型切换 |

---

## §4 网络调研摘要（权威要点）

### 4.1 运动学自行车模型（Kinematic Bicycle）

**后轴参考（推荐与当前 Pose 一致）：**

\[
\dot{x}=v\cos\theta,\quad
\dot{y}=v\sin\theta,\quad
\dot{\theta}=\frac{v}{L}\tan\delta
\]

- \(L\)：轴距（wheelbase）  
- \(v\)：纵向速度  
- \(\delta\)：前轮舵角（限幅）  
- 假设无侧滑；低速 AGV/停车场场景合理  

**来源：**

- ArXiv 教程 [1803.03758](https://ar5iv.labs.arxiv.org/html/1803.03758)（后轴参考、Pfaffian 约束）  
- [ros2_controllers mobile_robot_kinematics](https://github.com/ros-controls/ros2_controllers/blob/iron/doc/mobile_robot_kinematics.rst)（Car-Like / Bicycle 前向运动学）  
- CMU Snider 技术报告：[Automatic Steering Methods for Autonomous Automobiles](https://publications.ri.cmu.edu/storage/publications/pub_files/2009/2/Automatic_Steering_Methods_for_Autonomous_Automobile_Path_Tracking.pdf)（Pure Pursuit / Stanley 几何推导）

### 4.2 跟踪：Pure Pursuit vs Stanley

| 方法 | 要点 | Phase 4 建议 |
|------|------|----------------|
| Pure Pursuit | 后轴→前瞻点圆弧；曲率 \(κ=2\sinα/ℓ_d\)；\(\delta=\arctan(κ L)\) | **优先**：复用现有 tracker，加曲率→舵角适配 |
| Stanley | 前轴横向误差 + 航向误差；收敛更快 | 可选第二 tracker；有单测再合入 |
| MPC | 约束优化 | **不做** |

文献共识：Stanley 常优于 PP 的跟踪误差；PP 更简单稳。FleetSim Phase 4 以 **可切换模型 + PP→舵角** 为 MVP。

### 4.3 Hungarian vs Greedy（任务分配）

- **Greedy**：最近 idle 车接最近任务；快，局部最优（Phase 3 已有）  
- **Hungarian**：代价矩阵全局最优匹配；多项式时间；任务数≠车数时需补零/虚拟节点  

**来源：** [Wikipedia Hungarian algorithm](https://en.wikipedia.org/wiki/Hungarian_algorithm)；多 AGV 调度论文常用 Hungarian 做全局分配、Greedy 做快速/重调度前端（如 [PLOS One PSO+Hungarian](https://journals.plos.org/plosone/article?id=10.1371%2Fjournal.pone.0321616)）。

Phase 4：**实现 `HungarianAssigner : ITaskAssigner`**，`SchedulingModule` 可注入；对比单测证明代价更优或不劣于 Greedy 在构造算例上。

### 4.4 ROS2 桥接最佳实践

- Domain **不**依赖 `rclcpp`；桥在独立 ament 包中 `target_link_libraries(FleetSimDomain)`  
- 导出头文件到 `include/`；`ament_export_targets` / `ament_export_dependencies`（见 [ROS2 C++ library tutorial](https://ros2-tutorial.readthedocs.io/en/humble/cpp/cpp_library.html)）  
- Qt 与 ROS executor **分线程**；FleetSim 主工程默认 `FLEETSIM_BUILD_ROS2=OFF`，无 ROS 环境仍可编 Qt 仿真  
- MVP topic 建议：`fleetsim/pose`（或 per-vehicle）、`fleetsim/path`、`fleetsim/task_status`；payload 可用 `std_msgs/String` JSON（与现有 EventBus 一致）降低 msg 定义成本，后续再换自定义 msg

---

## §5 设计决策建议（执行前写 ADR）

### ADR-009 — Bicycle 与控制指令

**建议决策：**

1. 扩展 `ControlCommand`：
   - 保留 `linear_velocity`
   - 增加 `steering_angle`（rad）；DiffDrive 忽略之，继续用 `angular_velocity`
   - 或：引入 `VehicleModelType` + tracker 适配器，避免破坏旧测
2. `IVehicleModel` 接口：`integrate(pose, cmd, dt) → pose`
3. `DiffDriveModel` / `BicycleModel` 实现该接口
4. `Vehicle` 持有 `unique_ptr<IVehicleModel>` 或枚举工厂

### ADR-010 — ROS2 桥

**建议决策：**

- 桥订阅 Domain `EventBus` 字符串 topic，或薄 `Ros2Bridge` 在 App 可选编译单元  
- 主 CMake：`option(FLEETSIM_BUILD_ROS2 "Build ROS2 bridge" OFF)`  
- 文档：`docs/ros2_bridge.md` 写清 Humble/Jazzy、Windows 限制（若仅 Linux CI 则注明）

---

## §6 建议新增/变更文件清单

### Domain / Core

| 文件 | 职责 |
|------|------|
| `src/core/types/ControlCommand.h` | 扩展字段（兼容） |
| `src/domain/vehicle/IVehicleModel.h` | 模型接口 |
| `src/domain/vehicle/BicycleModel.h/.cpp` | 运动学积分 |
| `src/domain/vehicle/Vehicle.*` | 可切换模型 |
| `src/domain/control/SteeringAdapter.*` 或扩展 PurePursuit | κ→δ |
| `src/domain/scheduling/HungarianAssigner.h/.cpp` | 全局分配 |
| 扩展 `SchedulingModule` | 注入 assigner |

### App / UI

| 文件 | 职责 |
|------|------|
| `src/ui/dialogs/SettingsDialog.h/.cpp` | 模型/参数 |
| `src/app/SimController` / `ProjectManager` | 读写 settings / model |

### 桥与导出

| 路径 | 职责 |
|------|------|
| `cmake/FleetSimDomainConfig.cmake.in` | find_package 支持 |
| `bridges/ros2/` 或 `ros2_ws/src/fleetsim_bridge/` | rclcpp 节点 |
| `docs/ros2_bridge.md` | 构建说明 |
| `examples/domain_smoke/`（可选） | 无 Qt 链接 Domain 的 main |

### 测试

```
tests/domain/BicycleModelTest.cpp
tests/domain/HungarianAssignerTest.cpp
tests/domain/SteeringAdapterTest.cpp   # 若拆文件
tests/integration/BicycleScenarioTest.cpp
```

### 资产

```
assets/scenarios/bicycle_demo/map.json
assets/scenarios/bicycle_demo/scenario.json   # model: bicycle, wheelbase
```

---

## §7 如何继续开发（Goal 执行节奏）

1. **拉最新 main**，确认 Phase 3 在用户机器上已 Build 通过。  
2. **会话 0**：只写 ADR + 接口 stub + 测试红灯（TDD 可选）。  
3. **会话 1–2**：Bicycle 绿测 → 接入 SimEngine → 场景可玩。  
4. **会话 3**：CMake export；用无 Qt 的小 executable 验证链接。  
5. **会话 4**：ROS2 可选包；默认 OFF 不影响主工程。  
6. **会话 5**：SettingsDialog + Hungarian + 文档 + **双 Reviewer 子 agent** + push。  
7. 每会话结束：`SESSION_LOG` + 用户验证清单 + `git push`。  
8. **用户本地**：Qt Creator Configure + Build + Run `FleetSimTests`；有 ROS 再编桥。

### 子 Agent 监督协议（Goal 模式写进任务）

| 角色 | 输入 | 输出 | 否决权 |
|------|------|------|--------|
| Architect | Phase 目标 | mini-plan + NOT DO | 范围蔓延时否决 |
| Implementer | mini-plan | 代码 + 测试 | — |
| Reviewer-Code | diff + §10 | PASS/FAIL 列表 | FAIL 阻止 complete |
| Reviewer-Test | 测试目录 | 覆盖缺口 | 无单测即 FAIL |
| Reviewer-UI | UI diff | 是否违反一功能一面板 | FAIL |
| Scribe | 全会话 | SESSION_LOG + 四段式回复 | 模板不全即未完成 |

**实现建议**：每个会话结束用 Cursor `Task` 拉起 `explore`/`generalPurpose` 做 Reviewer；主 Agent 不得自评 PASS。

---

## §8 Phase 2/3 防回归（Phase 4 继续生效）

| ID | 问题 | 对策 |
|----|------|------|
| P2-01 | App 裸 `map::` | 一律 `domain::` 前缀 |
| P2-02 | .cpp 有实现 .h 无声明 | 先 .h 后 .cpp；grep 对齐 |
| P2-03 | lambda 漏 capture | UI Review 必查 |
| P2-04 | MinGW 大文件 | `-Wa,-mbig-obj` |
| P2-05 | `*.cmake` 被 gitignore | 白名单 `!cmake/...` |
| P2-06 | Agent 无 Qt | 用户本地 Build；交付检查项 |
| P2-07 | MainWindow 膨胀 | Settings 用 Dialog；逻辑不进 MainWindow |
| P3-CMake | include/link 被删光 | **每次改 CMake 后人工 diff 确认 target_* 仍在** |
| P3-Qt-fwd | 命名空间内 `class QTableWidget` | Qt 前向声明放全局命名空间 |

---

## §9 关键算法 / 配置默认值

| 参数 | 默认 | 说明 |
|------|------|------|
| wheelbase_m | 0.8–1.0 | 与 SVG 长度协调 |
| max_steering_rad | ~0.6 | ≈34° |
| pure_pursuit_lookahead_m | 沿用 Phase1 | Bicycle 时 δ=atan(κL) |
| assigner | greedy | settings 可切 hungarian |
| FLEETSIM_BUILD_ROS2 | OFF | 默认 |

---

## §10 禁止偷懒清单（Reviewer 打印打勾）

1. 禁止 Bicycle = DiffDrive 换皮  
2. 禁止 Domain 含 Qt / rclcpp  
3. 禁止 Settings 塞进 ControlPanel  
4. 禁止不做静态库 export  
5. 禁止 ROS2 默认 ON 搞挂无 ROS 构建  
6. 禁止新 Domain 类无 GTest  
7. 禁止删掉 CMake `target_include_directories` / `target_link_libraries`  
8. 禁止实现 MPC / 完整 Autoware  
9. 禁止 SESSION_LOG 缺「没做什么」  
10. 禁止不加 CMakeLists 就加源文件  
11. 禁止跳过双 Reviewer  
12. 禁止回退时间窗口避碰到距离判碰  

---

## §11 测试策略（足够测试的最低标准）

| 层级 | 要求 |
|------|------|
| 单元 | Bicycle：直行、定舵圆周、舵角限幅、零速；Hungarian：方阵/非方阵、与 Greedy 同代价算例 |
| 回归 | 现有 DiffDrive / MultiAgv / PathReservation 全绿 |
| 集成 | bicycle 场景 load→plan→N tick→pose 变化符合转弯方向 |
| 桥 | 若开启 ROS2：至少 1 个 publish smoke（可用 mock 或文档化手动测） |
| 突变 | `MUTATION_CHECKLIST` 增加 M24+（如 tan(δ) 符号反、wheelbase=0 未防护） |

---

## §12 用户本地验证清单（Agent 每次交付必须附上）

1. `git pull origin main`  
2. Qt Creator：**重新 Configure** → Build（MinGW Debug）  
3. 运行 `FleetSimTests`（关注 Bicycle* / Hungarian* / MultiAgv*）  
4. Open `assets/scenarios/bicycle_demo`（或文档指定路径）→ Play  
5. Settings：切换 diff_drive / bicycle，观察转弯差异  
6. （可选）按 `docs/ros2_bridge.md` 编译桥  
7. （可选）`tools/run_quality.ps1`  

**Agent 禁止**在中文用户路径下强行 cmake/ninja 导致环境报错；修复编译错误时以用户粘贴的编译日志为准。

---

## §13 风险与缓解

| 风险 | 缓解 |
|------|------|
| ControlCommand 破坏旧测试 | 字段默认 0；DiffDrive 忽略 steering |
| Windows 无 ROS2 | 桥可选；主工程 OFF |
| Bicycle + 旧 PP 振荡 | 限舵 + 前瞻随速；单测圆周 |
| Hungarian 数值不稳 | 用 double 代价；补虚拟节点文档化 |
| CMake export 复杂 | 先 `install(TARGETS ... EXPORT)` + 短文档 |

---

## §14 参考链接（执行 Agent 可再查）

1. https://ar5iv.labs.arxiv.org/html/1803.03758  
2. https://github.com/ros-controls/ros2_controllers/blob/iron/doc/mobile_robot_kinematics.rst  
3. https://publications.ri.cmu.edu/storage/publications/pub_files/2009/2/Automatic_Steering_Methods_for_Autonomous_Automobile_Path_Tracking.pdf  
4. https://en.wikipedia.org/wiki/Hungarian_algorithm  
5. https://ros2-tutorial.readthedocs.io/en/humble/cpp/cpp_library.html  
6. https://journals.plos.org/plosone/article?id=10.1371%2Fjournal.pone.0321616  

---

## §15 新对话开场白（短版，可与 §0 连用）

> 执行 `docs/PHASE4_GOAL_PROMPT.md`。Goal：完成 Phase 4（Bicycle + Domain 静态库导出 + ROS2 可选桥 + SettingsDialog + 建议 Hungarian）。严格子 Agent 互相监督；足够 GTest；每会话 push；不改 plan 文件；我不代跑中文路径 Build——交付验证步骤给我。从会话 0 开始，把 todos 标 in_progress/completed，直到验收清单全勾选。
