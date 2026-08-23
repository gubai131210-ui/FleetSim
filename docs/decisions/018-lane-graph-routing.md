# ADR 018: LaneGraph + Hybrid Routing

## 状态

草案（2026-08-23，Phase 8 Session 0）

## 背景

Phase 2 引入 `LaneGraph` 空 stub；Phase 5–7 完成栅格自由空间 A* / Hybrid A*、ST/MPC/预测与 Algorithm Workbench。工业栈中 **Route Server**（Nav2）在预定义 lane/route 图上求路，起终点不在图上时用自由空间 planner 做 **First/Last Mile**。FleetSim Phase 8 以 JSON `lanes[]` 实现轻量有向图 + `routing_mode` 切换，**不**引入 Lanelet2 / nav2_route。

## 决策

### 1. map.json `lanes` schema（MVP，写死）

```json
"lanes": {
  "nodes": [
    { "id": "n0", "x": 1.0, "y": 2.0 },
    { "id": "n1", "x": 5.0, "y": 2.0 }
  ],
  "edges": [
    { "from": "n0", "to": "n1", "bidirectional": false }
  ]
}
```

- 空车道：`"lanes": { "nodes": [], "edges": [] }` 或省略 `lanes`（加载时视为空图）。
- 边权：欧氏距离（节点坐标），教学默认。
- `bidirectional: true` 时自动添加反向边（同权）。

### 2. Domain 类型（`map/LaneTypes.h`）

| 类型 | 字段 |
|------|------|
| `LaneNode` | `id`, `x`, `y` |
| `LaneEdge` | `from`, `to`, `bidirectional` |
| `LaneMapData` | `nodes[]`, `edges[]` |
| `LanePath` | `std::vector<std::string>` 节点 id 序列 |

### 3. LaneGraph API

```cpp
class LaneGraph {
public:
    bool loadFromMap(const LaneMapData& lanes);
    std::optional<std::vector<std::string>> shortestPath(
        const std::string& from_id, const std::string& to_id) const;
    std::string nearestNodeId(double x, double y) const;
    core::Path centerlinePath(const std::vector<std::string>& node_ids) const;
    bool empty() const;
    std::size_t nodeCount() const;
    std::size_t edgeCount() const;
};
```

- 最短路径：**Dijkstra**（非负边权）；Session 1 实现。
- `nearestNodeId`：欧氏最近节点；snap 半径由 SimEngine / scenario 配置（默认 1.0 m）。
- `centerlinePath`：按节点序列连接折线 waypoint（可接 `DouglasPeuckerSmoother`）。

### 4. LaneRouter

```cpp
class LaneRouter {
public:
    explicit LaneRouter(const LaneGraph& graph);
    std::optional<core::Path> route(
        const std::string& from_node_id, const std::string& to_node_id) const;
};
```

`route` = `shortestPath` → `centerlinePath`；空图或不可达返回 `std::nullopt`。

### 5. SimEngine `routing_mode`

| 值 | 行为 |
|----|------|
| `freespace`（**默认**） | Phase 7 行为：A* / Hybrid A* 栅格规划 |
| `lane_graph` | 起终点 snap 到最近节点 → graph 路径 → centerline |
| `hybrid` | First mile（freespace → entry node）+ lane mile + last mile（→ goal） |

- scenario 字段：`simulation.routing_mode`（字符串枚举）。
- **禁止**：字段存在但 `planPath` 仍只调自由空间 planner。

### 6. 禁止事项

- `loadFromMap` 成功且传入非空 `nodes` 时 `nodeCount()` 不得恒为 0。
- 禁止 Domain 层 `#include <Qt*>` / rclcpp。
- 禁止完整 Lanelet2 / Nav2 Route Server 移植。

## 后果

- `MapSerializer` / `MapDocument` 扩展 `lanes`（Session 2）。
- 新增 GTest：`LaneGraphTest`、`LaneRouterTest`、`FirstLastMileIntegrationTest`、`MapSerializerLaneTest`。
- 默认 `freespace` 保证 Phase 7 回归不受影响。

## 参考

- [Nav2 Route Server](https://docs.nav2.org/)
- [Lanelet2 Routing](https://fzi-forschungszentrum-informatik.github.io/Lanelet2/lanelet2_routing/)
- ADR-002 map.json 基础格式
