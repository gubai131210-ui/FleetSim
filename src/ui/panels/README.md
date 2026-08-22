# UI Panels 目录

每个功能面板独立一个 Widget，由 `MainWindow` 通过 `QDockWidget` 或独立 `QDialog` 挂载。

## 已有面板

| 面板 | 文件 | 职责 |
|------|------|------|
| ControlPanel | `ControlPanel.*` | 仿真播放/暂停/单步/倍率 | Phase 1 |
| MonitorPanel | `MonitorPanel.*` | 误差/速度曲线 | Phase 2 |
| MapEditorPanel | `MapEditorPanel.*` | 地图编辑 | Phase 2 |
| TaskPanel | `TaskPanel.*` | 任务列表与 Add Task | Phase 3 |
| VehicleInfoPanel | `VehicleInfoPanel.*` | 选中车辆详情 | Phase 3 |
| SettingsDialog | `dialogs/SettingsDialog.*` | 模型/轴距/分配器 | Phase 4 |

## 计划面板

| 面板 | 计划文件 | Phase | 禁止 |
|------|---------|-------|------|
| SettingsDialog | `SettingsDialog.*` | 4 ✅ | 禁止堆在 ControlPanel |
| PlannerTrackerDialog（或扩展 Settings） | `dialogs/PlannerTrackerDialog.*` | 5 | 禁止堆在 ControlPanel；禁止塞进 MainWindow |

## 新增面板流程

1. 在本目录新建 `XxxPanel.h/.cpp`
2. 更新 `src/ui/CMakeLists.txt`
3. 在 `MainWindow` 仅添加 dock / 菜单挂载（≤10 行）
4. 在 `SESSION_LOG.md` 记录新面板路径
