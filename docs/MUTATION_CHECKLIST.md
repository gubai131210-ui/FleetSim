# Domain 层变异测试清单（手工）

> 每条：在源码中做微小改动 → 运行 `FleetSimTests` → 预期至少一个测试 **FAIL**。  
> 若仍全部 PASS，说明测试不足，需补测并在 SESSION_LOG 记录。

| ID | 文件 / 函数 | 变异操作 | 预期失败测试 |
|----|-------------|----------|--------------|
| M01 | `AStarPlanner.cpp` / 对角切角 | 注释掉 corner-cut 检查 | `AStarPlannerTest` |
| M02 | `PurePursuitTracker.cpp` / forward dot | 删除 `forward dot <= 0 continue` | `PurePursuitTrackerTest` |
| M03 | `OccupancyGrid.cpp` / inflate | `radius_m * 0.5` | `OccupancyGridTest` |
| M04 | `DouglasPeuckerSmoother.cpp` | `epsilon_m * 2` | 相关 smoother 测试 |
| M05 | `AStarPlanner.cpp` / 启发式 | 将 octile 改为曼哈顿 | `AStarPlannerTest` |
| M06 | `MapSerializer.cpp` / rect 边界 | 去掉 `width > 0` 校验 | `MapSerializerTest` |
| M07 | `MapSerializer.cpp` / polygon | 允许 `< 3` 顶点通过 | `MapSerializerTest` |
| M08 | `ScenarioSerializer.cpp` / vehicles | 忽略 `id` 字段 | `ScenarioSerializerTest` |
| M09 | `OccupancyGrid.cpp` / isInside | 边界 `>=` 改 `>` | `OccupancyGridTest` |
| M10 | `PurePursuitTracker.cpp` / lookahead | lookahead 减半 | `PurePursuitTrackerTest` |
| M11 | `DiffDriveModel.cpp` / clamp | 移除速度 clamp | `VehicleTest` |
| M12 | `MapLoader.cpp` / inflation | 跳过 inflate 调用 | `MapLoaderTest` |
| M13 | `AStarPlanner.cpp` / 8-connect | 禁用对角邻居 | `AStarPlannerTest` |
| M14 | `ProjectManager.cpp` (app) / save | 不写 `map.json` | `ProjectManagerTest` |
| M15 | `ScenarioSerializer.cpp` / dt | `dt_s` 写为负数 | `ScenarioSerializerTest` |
| M16 | `OccupancyGrid.cpp` / worldToCell | 行列交换 | `OccupancyGridTest` |
| M17 | `DouglasPeuckerSmoother.cpp` | 返回原始未平滑路径 | `AStarPlannerTest` 或集成 |
| M18 | `MapSerializer.cpp` / 空障碍 | 丢弃 obstacles 数组 | `MapSerializerTest` |
| M19 | `PurePursuitTracker.cpp` | 到达阈值改大 10 倍 | `PurePursuitTrackerTest` |
| M20 | `SimEngine.cpp` / goal distance | 到达阈值 `0.2` 改 `0.001` | `SimEngineTest` |
| M21 | `GreedyAssigner.cpp` / 距离 | 选最远车而非最近 | `GreedyAssignerTest` |
| M22 | `PathReservationTable.cpp` / 冲突 | 永远返回无冲突 | `PathReservationTableTest` |
| M23 | `TaskQueue.cpp` / markDone | 不更新 status | `TaskQueueTest` |
| M24 | `BicycleModel.cpp` / tan(δ) | 将 `tan(δ)` 符号取反 | `BicycleModelTest` |
| M25 | `BicycleModel.cpp` / wheelbase | 去掉 `wheelbase>0` 防护 | `BicycleModelTest.InvalidWheelbase*` |
| M26 | `SteeringAdapter.cpp` / atan | `atan(κL)` 改为 `κL` | `SteeringAdapterTest` |
| M27 | `HungarianAssigner.cpp` / 代价 | 最大化代价而非最小化 | `HungarianAssignerTest` |

## 执行记录模板

```
日期:
执行人:
结果: M01 PASS/FAIL, M02 ...
需补测: ...
```
