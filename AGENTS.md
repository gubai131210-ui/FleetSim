# FleetSim Agent 协作规范

## 项目概述

多 AGV 调度与控制系统 2D 仿真平台。C++17 + Qt 6 Widgets + QGraphicsView + CMake。

## 硬规则（违反 = Review 不通过）

1. **Domain 层禁止 `#include <Qt*>`** — 保证可抽离为 ROS2 静态库
2. **每次会话必须更新 SESSION_LOG.md** — 按 `docs/AGENT_SESSION_TEMPLATE.md` **完整填写**，禁止空写
3. **每次回复用户必须包含**：本次做了什么 / 没做什么 / 为什么 / 本地验证步骤
4. **接口先于实现** — 新模块先写 `.h` + 单测，再写 `.cpp`
5. **不改无关文件** — 本次 scope 外的代码不动
6. **UI 禁止堆控件** — 见 `docs/UI_GUIDELINES.md` 与 ADR-005
7. **必须写单测** — Core / Domain 每个新类至少 1 个 test
8. **禁止跳过 Reviewer** — 每次会话末尾跑 Reviewer 子 agent
9. **一文件一职责** — 见下方「文件组织规范」
10. **车辆外观用 SVG 资源** — 见 `docs/decisions/004-vehicle-rendering.md`

## UI 硬规则（Reviewer 必查）

| 禁止 | 必须 |
|------|------|
| 在 MainWindow 添加业务 QPushButton | 新建 `src/ui/panels/XxxPanel.*` |
| 用 QToolBar 承载仿真/编辑/监控控件 | 用 QDockWidget 挂载 Panel |
| 把 QCustomPlot / 表格 / 表单塞 centralWidget | centralWidget 只能是 MapView |
| 在一个 Panel 混合仿真控制 + 监控 + 任务 | 一功能一面板，见 UI_GUIDELINES |
| MainWindow.cpp 超过 150 行 | 拆到 panels/ 或 app/ |

## 模块边界

| 层 | 目录 | 可依赖 | 禁止依赖 |
|----|------|--------|---------|
| Core | `src/core/` | STL, Eigen | Qt, Domain |
| Domain | `src/domain/` | Core, Eigen, nlohmann/json | Qt |
| App | `src/app/` | Domain | UI 控件 |
| UI | `src/ui/` | App, Domain(只读类型), Qt | — |

## 文件组织规范（可持续开发）

| 规则 | 说明 |
|------|------|
| **一类一文件** | 每个公开类独立 `.h` + `.cpp`，禁止多个类挤在同一文件 |
| **行数上限** | 单文件建议 ≤ 300 行；超过 500 行必须拆分 |
| **MainWindow** | 仅负责布局编排、菜单、Dock 挂载，**不含**仿真/规划/控制逻辑 |
| **SimEngine** | 只做 tick 编排，具体算法在子模块 |
| **UI 子目录** | `ui/map/` 地图；`ui/graphics/` 图元；`ui/panels/` 面板；`ui/dialogs/` 对话框；`ui/editor/` 编辑器 |
| **Domain 子目录** | `map/`, `planning/`, `control/`, `vehicle/`, `scheduling/`, `collision/` |
| **资源与代码分离** | 车辆 SVG → `assets/vehicles/`；场景 → `assets/scenarios/` |

## 命名规范

- 类名：`PascalCase`（`SimEngine`, `PurePursuitTracker`）
- 接口：`I` 前缀（`IPathTracker`, `ITaskAssigner`）
- 成员变量：`snake_case_`（尾部下划线）
- 文件名：与类名一致
- 测试：`<ClassName>_<Scenario>Test`

## 仿真约定

- 坐标：米制 `(x, y, θ_rad)`，原点在地图左下
- 时间：`SimClock` 固定 dt 默认 `0.05s` (20Hz)
- 栅格：默认分辨率 `0.1 m/cell`

## 会话流程（4 角色）

1. 读 `AGENTS.md` + `SESSION_LOG.md` + `docs/AGENT_SESSION_TEMPLATE.md`
2. **Architect** → mini-plan（含 **允许改动** 与 **明确 NOT DO**）
3. **Implementer** → 代码 + 单测（限定 scope）
4. **Reviewer** → 对照硬规则 + UI_GUIDELINES + 偷懒自检清单
5. **Scribe** → 按模板更新 `SESSION_LOG.md`
6. **回复用户** → 做了什么 / 没做什么 / 为什么 / 本地验证
7. 用户本地编译测试（中文路径，Agent 不代跑编译）

## 关键文档索引

| 文档 | 用途 |
|------|------|
| `docs/AGENT_SESSION_TEMPLATE.md` | 会话交付强制格式 |
| `docs/UI_GUIDELINES.md` | UI 面板规范 |
| `docs/decisions/005-ui-page-structure.md` | UI 架构 ADR |
| `docs/DEVELOPMENT_PLAN.md` | Phase 计划 + 禁止偷懒清单 |

## 当前 Phase

→ 见 `SESSION_LOG.md` 最新条目

## 决策记录

架构与格式决策见 `docs/decisions/`。变更架构必须先新增 ADR。
