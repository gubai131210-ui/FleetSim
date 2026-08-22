# ADR 005: UI 页面与面板结构

## 状态

已接受（2026-08-22）

## 背景

Agent 容易把按钮、控件堆进 `MainWindow` 或 `QToolBar`，导致不可维护。需要强制「一功能一面板」。

## 决策

### MainWindow 职责（仅限）

1. 设置窗口标题、尺寸
2. `setCentralWidget(MapView)` — 中央永远是地图画布
3. 用 `QDockWidget` 挂载各功能面板
4. 菜单栏：`View` → 显示/隐藏各 Dock
5. 连接面板的 `statusMessage` 到 `statusBar`

**MainWindow 禁止**：仿真算法、地图编辑逻辑、超过 3 个 `QPushButton`、业务相关的 lambda。

### 面板 vs 页面

| 类型 | 何时用 | 实现 |
|------|--------|------|
| **Dock 面板** | 仿真中常显/可隐藏的功能 | `QDockWidget` + `XxxPanel` |
| **独立对话框** | 配置向导、关于、导入导出 | `QDialog` 子类 |
| **Stacked 页面** | 互斥的模式（编辑 vs 运行） | `QStackedWidget` 在独立 `ModeSwitcher` 中 |

### 目录约定

```
src/ui/
├── MainWindow.*          # 仅编排
├── map/                  # 地图视图
├── graphics/             # 图元
├── panels/               # 所有 Dock 面板 ← 新 UI 功能放这里
└── dialogs/              # 独立对话框（Phase 2+）
```

### 控件数量红线

| 位置 | 允许 |
|------|------|
| MainWindow | ≤ 0 个业务按钮（菜单项除外） |
| 单个 Panel | 专注单一职责；超过 8 个控件考虑拆面板 |
| QToolBar | **禁止**添加业务控件（已废弃 Simulation toolbar） |

## 示例布局

```
┌─────────────────────────────────────────────┬──────────────┐
│                                             │ ControlPanel │
│              MapView (central)              │  (dock right)│
│                                             │              │
│                                             │ MonitorPanel │
│                                             │  (dock bot)  │
└─────────────────────────────────────────────┴──────────────┘
```

## 后果

- Phase 0 的 Simulation QToolBar 已迁移至 `ControlPanel`（dock）
- 后续 Monitor / Editor / Task 必须新建面板文件
