# FleetSim UI 设计规范

> Reviewer 检查 UI 变更时对照本文档。违反任一条 = Review FAIL。

## 核心原则

1. **一功能一面板** — 新功能 = 新文件 in `src/ui/panels/` 或 `src/ui/dialogs/`
2. **MainWindow 是壳** — 只挂载，不实现
3. **中央永远是 MapView** — 不把表单、表格塞进 centralWidget
4. **禁止 QToolBar 堆业务按钮** — 控件放在对应 Panel 里

## 什么时候必须新建页面/面板？

| 场景 | 正确做法 | 错误做法（禁止） |
|------|---------|----------------|
| 仿真控制（播放/暂停） | `ControlPanel` | MainWindow 里 addToolBar |
| 误差/速度曲线 | 新建 `MonitorPanel` + dock bottom | 在 ControlPanel 底部塞 QCustomPlot |
| 地图障碍物编辑 | 新建 `MapEditorPanel` + 编辑模式 | 在 MapView 上叠 10 个按钮 |
| 任务列表与分配 | 新建 `TaskPanel` | toolbar 加「Add Task」 |
| 车辆属性查看 | 新建 `VehicleInfoPanel` | statusBar 显示全部信息 |
| 项目打开/保存 | 新建 `ProjectDialog` | MainWindow 构造函数里 QFileDialog |
| 设置（dt、栅格分辨率） | 新建 `SettingsDialog` | 在 ControlPanel 塞 20 个 QSpinBox |

## 文件模板

新建面板 checklist：

- [ ] `src/ui/panels/XxxPanel.h`
- [ ] `src/ui/panels/XxxPanel.cpp`
- [ ] 更新 `src/ui/CMakeLists.txt`
- [ ] `MainWindow` 中添加 `QDockWidget` 挂载（≤15 行）
- [ ] `SESSION_LOG.md` 记录新文件

## MainWindow 允许代码示例

```cpp
// ✅ 正确：挂载面板
auto* control_dock = new QDockWidget(tr("Control"), this);
control_dock->setWidget(new ControlPanel(sim_controller_, control_dock));
addDockWidget(Qt::RightDockWidgetArea, control_dock);

// ❌ 禁止：业务按钮
addToolBar(...)->addAction("Play", this, [this]{ ... });
```

## 面板内部规范

- 用 `QGroupBox` 分组相关控件
- 布局用 `QVBoxLayout` / `QHBoxLayout`，禁止绝对定位
- 面板通过 **signal** 与 MainWindow / SimController 通信，不直接访问全局单例
- 单个 `.cpp` 不超过 300 行，超过则拆成子 Widget

## Phase 面板路线图

| Phase | 新建面板 |
|-------|---------|
| 1 | （复用 ControlPanel；路径渲染在 MapView） |
| 2 | `MonitorPanel`, `MapEditorPanel`, `ProjectDialog` |
| 3 | `TaskPanel`, `VehicleInfoPanel` |
| 4 | `SettingsDialog`（模型切换） |
| 7 | `AlgorithmWorkbenchDialog` + `ExperimentComparePanel` |
| 8 | **`LaneEditorPanel`**（车道拓扑）、**`RoutingPage`**（Workbench 第 5 页）；禁止堆进 MapEditor/Monitor |
| 9 | **`BehaviorTreePanel`**（BT 监控 dock）、**`BehaviorPage`**（Workbench 第 6 页）；ComparePanel 仅增 CSV 导出；禁止堆进 Routing/Monitor |
| 10 | **`OsmImportPanel` / `MultiAgentBehaviorPanel`**；Workbench **MapImportPage / BehaviorXmlPage**；禁止堆进 MapEditor/LaneEditor/BehaviorPage |
| 11 | **`PerceptionPanel`**；Workbench **PerceptionPage / MultiModalPredictionPage / RegulatoryPage / NonlinearControlPage**；Coordination 仅增 eecbs；禁止堆进 Control/Speed/MapImport |

## SVG 车辆

车辆外观不在 Panel 里画，用 `assets/vehicles/*.svg` + `VehicleGraphicsItem`。见 ADR-004。
