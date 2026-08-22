# ADR 002: 地图与场景文件格式

## 状态

已接受（2026-08-22）

## 决策

地图与场景分离，JSON 格式，使用 nlohmann/json 解析。

```
assets/scenarios/<name>/
├── map.json       # 静态环境：尺寸、栅格分辨率、障碍物
└── scenario.json  # 动态：AGV 初始位姿、任务、仿真参数
```

### map.json 结构（草案）

```json
{
  "version": 1,
  "width_m": 20.0,
  "height_m": 15.0,
  "grid_resolution_m": 0.1,
  "obstacles": [
    { "type": "rect", "x": 5.0, "y": 3.0, "width": 2.0, "height": 1.0 },
    { "type": "polygon", "vertices": [[1,1],[3,1],[2,3]] }
  ],
  "lanes": []
}
```

### scenario.json 结构（草案）

```json
{
  "version": 1,
  "simulation": { "dt_s": 0.05, "realtime": false },
  "vehicles": [
    {
      "id": "agv_0",
      "model": "diff_drive",
      "svg": "assets/vehicles/agv_diff.svg",
      "pose": { "x": 1.0, "y": 1.0, "theta": 0.0 }
    }
  ],
  "tasks": []
}
```

## 后果

- Git diff 友好，Agent 可读写
- Phase 2 实现 ProjectManager 读写
