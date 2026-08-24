# FleetSim → 自动驾驶学习路径

| FleetSim 阶段 | 仿真能力 | 自动驾驶对应概念 | 延伸 |
|--------------|---------|-----------------|------|
| Phase 1 | A* + Pure Pursuit + DiffDrive | 全局规划、几何跟踪 | Hybrid A*, MPC |
| Phase 2 | 地图编辑 + 监控曲线 | 高精地图、调试可视化 | Lanelet2 |
| Phase 3 | 多车调度 + 时间窗口避碰 | 多智能体、V2X 协同 | 拍卖算法、ST-Graph |
| Phase 4 | Bicycle + ROS2 桥接 | 乘用车控制、中间件 | nav2, Autoware |
| Phase 5 | Hybrid A* + Stanley + Priority/CBS-lite | 非完整约束规划、横向控制、MAPF | MPC、ST-Graph、EECBS |
| Phase 6 | 线性 MPC + ST-Graph 速度规划 | 预测控制、时空速度规划 | 完整 Autoware、感知 |
| Phase 7 | 常速预测 + ST 增强 + 实验工作台 UI | 动态障碍预测、仿真指标对比 | 神经网络预测、Behavior Tree ✅ |
| Phase 8 | LaneGraph + First/Last Mile + Lane 编辑 UI | 车道级路由、Nav2 Route 思想 | 完整 Lanelet2、BT 引擎（→ Phase 9） ✅ |
| Phase 9 | Behavior Tree MVP + CSV 导出 + Behavior UI 分层 | Nav2 BT 编排 plan/follow/replan/recovery | 完整 BT.CPP、Lanelet2 导入（→ Phase 10） ✅ |
| Phase 10 | OSM 子集导入 + BT XML/Spin·BackUp + 多车 BT + CBS-lite | HD Map、Nav2 Behavior Server、MAPF 约束树 | 完整 EECBS、感知、Groot2（→ Phase 11） ✅ |
| Phase 11 | Lidar 感知层 + 多模态预测 + EECBS 教学 + 交通灯 + 非线性 NMPC | Nav2 ObstacleLayer、IMM/CTRA、EECBS、监管要素、NMPC | CasADi 全栈、NN 预测、3D Voxel（→ Phase 12） |

Domain 层保持无 Qt。Phase 7–10 ✅。Phase 10：OSM + BT XML + CBS-lite + **OsmImportPanel / BehaviorXmlPage**。Phase 11 目标见 `docs/PHASE11_GOAL_PROMPT.md`。
