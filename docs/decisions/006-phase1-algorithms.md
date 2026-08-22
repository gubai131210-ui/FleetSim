# ADR 006: Phase 1 算法参数

## 状态

已接受（2026-08-22）

## 决策

Phase 1 采用以下默认参数：

| 模块 | 参数 |
|------|------|
| OccupancyGrid | resolution 0.1m；inflation 0.55m |
| A* | 8 连通；Octile 启发式；禁角切 |
| Smoother | Douglas-Peucker ε=0.15m；插值步长 0.2m |
| Pure Pursuit | lookahead=0.6m；v_max=0.5；ω_max=1.0；原地旋转阈值 π/4 |
| DiffDrive | 单轨模型；速度限幅 |
| Goal | tolerance 0.2m |

## 参考

- Nav2 Regulated Pure Pursuit: https://arxiv.org/pdf/2305.20026
- Grid A* with inflation: autonav-labyrinth, light_Astar_ROS2_path_planner
