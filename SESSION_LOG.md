# FleetSim Session Log

## 会话流程

1. 读 `AGENTS.md` + `docs/AGENT_SESSION_TEMPLATE.md` + 本文件最新条目
2. Architect → mini-plan（含 NOT DO 列表）
3. Implementer → 代码 + 单测
4. Reviewer → 硬规则 + UI_GUIDELINES + 偷懒自检
5. Scribe → **按模板完整填写**下方新条目
6. 回复用户：做了什么 / 没做什么 / 为什么 / 本地验证

> 新会话条目必须包含：✅已完成 / ❌未完成 / 🚫偷懒自检 / 文件清单

---

## [2026-08-22] 框架强化 — Agent 反偷懒 + UI 面板规范

### 本次 Scope（Architect 定义）
- 目标：强化 Agent 交付规范，防止 UI 堆控件；示范正确 Panel 拆分
- 允许改动：`docs/`, `AGENTS.md`, `.cursor/rules/`, `src/ui/panels/`, `MainWindow.cpp`
- 明确不在本次范围：Domain 仿真逻辑、Phase 1 算法、MonitorPanel 实现

### ✅ 已完成
- [x] `docs/AGENT_SESSION_TEMPLATE.md` — 强制交付模板（做了什么/没做什么/偷懒自检）
- [x] `docs/UI_GUIDELINES.md` — UI 面板规范与禁止行为表
- [x] `docs/decisions/005-ui-page-structure.md` — UI 架构 ADR
- [x] `src/ui/panels/ControlPanel.h/.cpp` — 仿真控制独立面板
- [x] `src/ui/panels/README.md` — 面板目录说明与 Phase 路线图
- [x] `MainWindow` 重构：移除 QToolBar，改用 QDockWidget 挂载 ControlPanel
- [x] `AGENTS.md` / `.cursor/rules/fleetsim.mdc` 强化硬规则
- [x] `docs/DEVELOPMENT_PLAN.md` 扩充禁止偷懒清单

### ❌ 未完成 / 故意不做
| 项目 | 原因 | 计划 |
|------|------|------|
| MonitorPanel | 属 Phase 2 | Phase 2 独立会话 |
| MapEditorPanel | 属 Phase 2 | Phase 2 独立会话 |
| TaskPanel | 属 Phase 3 | Phase 3 |
| ControlPanel 单测 | UI 层 Phase 0 不要求 | 可选 Phase 2 |

### 🚫 禁止偷懒自检
- [x] 没有把多个类挤进同一文件
- [x] 没有在 MainWindow 堆业务按钮（已迁到 ControlPanel）
- [x] 新 UI 使用了独立 Panel 文件
- [x] Domain 层无 Qt include
- [x] 本次无新 Domain 类，无需新单测
- [x] 未跨 Phase 实现功能
- [x] SESSION_LOG 本节已完整填写

### 新增/变更文件清单
| 文件 | 操作 |
|------|------|
| `docs/AGENT_SESSION_TEMPLATE.md` | 新增 |
| `docs/UI_GUIDELINES.md` | 新增 |
| `docs/decisions/005-ui-page-structure.md` | 新增 |
| `src/ui/panels/ControlPanel.*` | 新增 |
| `src/ui/panels/README.md` | 新增 |
| `src/ui/MainWindow.cpp` | 修改（Dock 化） |
| `AGENTS.md`, `.cursor/rules/fleetsim.mdc` | 修改 |

### Reviewer 结果
- PASS（MainWindow.h 已补全 setupDockPanels/setupViewMenu 声明）

### 用户本地验证
- Qt Creator 重新 Build → Run
- 预期：右侧 Dock「Control」面板含 Play/Pause/Step；无顶部 Simulation 工具栏；View 菜单可切换 Control Panel

### 下次会话建议
- Phase 1：OccupancyGrid + A*，仍不在 MainWindow 加控件

---

## [2026-08-22] Phase 0 — Harness 与工程骨架

**Architect 目标**：模块化 CMake + 核心骨架 + 文档 + SVG 渲染 ADR

**Implementer 完成**：

- [x] 重构 CMake 为 Core / Domain / App / UI / Tests 分层
- [x] FetchContent：Google Test、nlohmann/json、Eigen
- [x] Core：`SimClock`、`EventBus`、基础类型
- [x] Domain：`SimEngine` 骨架；`SchedulingModule`、`CollisionModule` stub
- [x] UI：`MapView`、`VehicleGraphicsItem`（SVG 加载 stub）
- [x] 文档：`AGENTS.md`、ADR 001–004、`learning-path.md`
- [x] 资源：示例 SVG、`map.json`、`scenario.json`
- [x] `.gitignore`、`.cursor/rules/fleetsim.mdc`

**接口变更**：

- 新增 `fleetsim::SimEngine::tick(double dt)`
- 新增 `fleetsim::EventBus::publish/subscribe`
- 新增 `fleetsim::ui::VehicleGraphicsItem`（SVG 车辆图元）

**Reviewer 结果**：PASS

**下次注意（Phase 1）**：

- 从 `OccupancyGrid` + A* 开始
- `MapView` 接 SimEngine EventBus 渲染
- Pure Pursuit + DiffDrive 在 Domain 层实现

**本地验证**（用户执行）：

```
在 Qt Creator 中打开项目，重新配置 CMake，Build → Run Tests
```
