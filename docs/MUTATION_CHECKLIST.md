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
| M28 | `HybridAStarPlanner.cpp` / 扩展步 | `motion_resolution` 符号取反或舵角全 0 | `HybridAStarPlannerTest` / `PlannerSwitch*` |
| M29 | `StanleyTracker.cpp` / k | `k_gain` 取反或忽略横偏项 | `StanleyTrackerTest` |
| M30 | `PriorityPathCoordinator` / 顺序 | 忽略高优先级，低优先级先占走廊 | `PriorityCoordinatorTest.LowerPriorityDetours*` |
| M31 | `MpcLateralTracker.cpp` / A,B | 将预测矩阵 A 或 B 置零 | `MpcLateralTrackerTest.PredictionOrCostNonTrivial` |
| M32 | `StGraphSpeedPlanner.cpp` / peers | `buildOccupancies` 忽略 peers（空列表） | `StGraphSpeedPlannerTest.ClearingPeerObstacles*` / `StGraphSimEngineWiringTest.WithPeers*` |
| M33 | `MpcLateralTracker.cpp` / Q,R | 强制 `q_lat=q_heading=r_steer=0` | `MpcLateralTrackerTest.PredictionOrCostNonTrivial` / `lastCostNonTrivial` |
| M34 | `ConstantVelocityPredictor.cpp` / horizon | 外推忽略 `horizon_s` 或零 horizon 仍长路径 | `ConstantVelocityPredictorTest.StraightLineExtrapolationLength` |
| M35 | `ConstantVelocityPredictor.cpp` / extrapolation | 不沿 heading 外推（单点或静态复制） | `ConstantVelocityPredictorTest.TurningNominalSpeedUsesHeading` / `StGraphWithPredictionTest` |
| M36 | `ExperimentMetrics.cpp` / recordTick | `recordTick` 空操作或 `summarize` 不聚合 | `ExperimentMetricsTest.RecordsSamplesAndAggregatesMeanAbsCrossTrack` |
| M37 | `LaneGraph.cpp` / rebuildAdjacency | 删除某条 `edges` 或忽略 `bidirectional` 反向边 | `LaneGraphTest.ShortestPathForkChoosesShorterRoute` / `MapSerializerLaneTest.BidirectionalEdge*` |
| M38 | `LaneGraph.cpp` / nearestNodeId | 恒返回第一个节点 id | `LaneGraphTest.NearestNodeIdPicksClosest` |
| M39 | `SimEngine.cpp` / planHybridPathForAgent | 跳过 lane 段，仅 first+last freespace | `FirstLastMileIntegrationTest.HybridPathIsLongerThanLaneGraphSegment` / `LaneRoutingDemoScenarioTest.HybridLaneGraphAndFreespaceAllPlan` |

## Phase 5 变异说明

- M28：证明 Hybrid 做了运动学扩展而非栅格 A* 换皮  
- M29：证明 Stanley 含前轴横偏项，非 Pure Pursuit 换皮  
- M30：证明 Priority 尊重高优先级轨迹占用  

## Phase 6 变异说明

- M31：证明 MPC 使用非平凡预测矩阵，非 Stanley 换皮  
- M32：证明 ST 消费他车 Path 占用，非距离停车 / 忽略 peers  
- M33：证明代价权重非平凡（Q/R），权重全 0 须被测抓住  

## Phase 7 变异说明

- M34：`ConstantVelocityPredictor.cpp` / horizon — 强制 `horizon_s <= 0` 仍返回多点或长度 \(\gg v\cdot T\) | `ConstantVelocityPredictorTest.StraightLineExtrapolationLength` / `ZeroHorizon*`  
- M35：`ConstantVelocityPredictor.cpp` — 忽略外推，仅返回 `{current.x, current.y}` 单点 | `ConstantVelocityPredictorTest.*` / `StGraphWithPredictionTest`  
- M36：`ExperimentMetrics.cpp` — `recordTick` 不写入 deque / `summarize` 永远 sample_count=0 | `ExperimentMetricsTest.*`  

## Phase 8 变异说明

- M37：`LaneGraph.cpp` — 建图时丢弃边或 `shortestPath` 恒 `nullopt` | `LaneGraphTest.*` / `LaneRouterTest.*`  
- M38：`LaneGraph.cpp` — `nearestNodeId` 忽略坐标 | `LaneGraphTest.NearestNodeIdPicksClosest`  
- M39：`SimEngine.cpp` — `hybrid` 不调用 `LaneRouter` / lane 段为空 | `FirstLastMileIntegrationTest.*` / `LaneRoutingDemoScenarioTest.*`  

## 执行记录模板

```
日期:
执行人:
结果: M01 PASS/FAIL, M02 ...
需补测: ...
```
