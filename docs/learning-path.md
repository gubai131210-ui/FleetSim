# FleetSim → 自动驾驶学习路径

| FleetSim 阶段 | 仿真能力 | 自动驾驶对应概念 | 延伸 |
|--------------|---------|-----------------|------|
| Phase 1 | A* + Pure Pursuit + DiffDrive | 全局规划、几何跟踪 | Hybrid A*, MPC |
| Phase 2 | 地图编辑 + 监控曲线 | 高精地图、调试可视化 | Lanelet2 |
| Phase 3 | 多车调度 + 时间窗口避碰 | 多智能体、V2X 协同 | 拍卖算法、ST-Graph |
| Phase 4 | Bicycle + ROS2 桥接 | 乘用车控制、中间件 | nav2, Autoware |
| Phase 5 | Hybrid A* + Stanley + Priority/CBS-lite | 非完整约束规划、横向控制、MAPF | MPC、ST-Graph、EECBS |
| Phase 6 | 线性 MPC + ST-Graph 速度规划 | 预测控制、时空速度规划 | 完整 Autoware、感知 |

Domain 层保持无 Qt。Phase 5 已交付 Hybrid/Stanley/Priority；Phase 6 在 Domain 内增强控制与纵向规划（仍零 Qt）。完整执行提示：`docs/PHASE6_GOAL_PROMPT.md`。
