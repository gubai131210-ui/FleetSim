# FleetSim → 自动驾驶学习路径

| FleetSim 阶段 | 仿真能力 | 自动驾驶对应概念 | 延伸 |
|--------------|---------|-----------------|------|
| Phase 1 | A* + Pure Pursuit + DiffDrive | 全局规划、几何跟踪 | Hybrid A*, MPC |
| Phase 2 | 地图编辑 + 监控曲线 | 高精地图、调试可视化 | Lanelet2 |
| Phase 3 | 多车调度 + 时间窗口避碰 | 多智能体、V2X 协同 | 拍卖算法、ST-Graph |
| Phase 4 | Bicycle + ROS2 桥接 | 乘用车控制、中间件 | nav2, Autoware |
| Phase 5 | Hybrid A* + Stanley + Priority/CBS-lite | 非完整约束规划、横向控制、MAPF | MPC、ST-Graph、EECBS |

Domain 层保持无 Qt，Phase 4 已导出静态库 / ROS2 桥；Phase 5 在 Domain 内增强规划与跟踪（仍零 Qt）。
