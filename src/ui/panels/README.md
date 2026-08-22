# UI Panels 目录

每个功能面板独立一个 Widget，由 `MainWindow` 通过 `QDockWidget` 或独立 `QDialog` 挂载。

## 已有面板

| 面板 | 文件 | 职责 |
|------|------|------|
| ControlPanel | `ControlPanel.*` | 仿真播放/暂停/单步 |

## 计划面板（Phase 1+，各自独立文件）

| 面板 | 计划文件 | Phase | 禁止 |
|------|---------|-------|------|
| MonitorPanel | `MonitorPanel.*` | 2 | 禁止与 ControlPanel 合并 |
| MapEditorPanel | `MapEditorPanel.*` | 2 | 禁止塞进 MainWindow |
| TaskPanel | `TaskPanel.*` | 3 | 禁止用 toolbar 按钮代替 |
| VehicleInfoPanel | `VehicleInfoPanel.*` | 3 | 禁止堆在 statusBar |

## 新增面板流程

1. 在本目录新建 `XxxPanel.h/.cpp`
2. 更新 `src/ui/CMakeLists.txt`
3. 在 `MainWindow` 仅添加 dock / 菜单挂载（≤10 行）
4. 在 `SESSION_LOG.md` 记录新面板路径
