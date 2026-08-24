# ADR 022: OSM/Lanelet2 教学子集导入

## 状态

草案（2026-08-24，Phase 10 Session 0；Session 1 实装 `OsmLaneletImporter`）

## 背景

Phase 8 以 JSON `lanes[]` 手工维护 `LaneGraph`。工业 Autoware 栈使用 Lanelet2 OSM（`.osm`）描述 HD Map。Phase 10 在 Domain 层自研**教学子集**解析器，**不**引入 Lanelet2 C++ 库或 GeographicLib 全投影栈。

## 决策

### 1. simulation / map 字段（scenario.json）

```json
"simulation": {
  "map_source": "json",
  "osm_path": "map.osm"
}
```

| 字段 | 默认 | 说明 |
|------|------|------|
| `map_source` | `json` | `json` 保留 Phase 8 `map.json` lanes；`osm` 从 `osm_path` 导入 |
| `osm_path` | 空 | 相对场景目录或 `assets/maps/` |

### 2. OSM 教学子集（写死）

支持元素：

| 元素 | 要求 |
|------|------|
| `node` | `local_x`/`local_y` 或 `x`/`y` tag；禁止仅 lat/lon 无本地坐标 |
| `way` | 有序 `nd ref` 节点引用 |
| `relation` | `tag type=lanelet`；成员 `role=left|right`（可选 `centerline`） |

**禁止**：regulatory_element、交通灯、停车位 Area、完整地理投影。

### 3. Domain API（`map/OsmLaneletImporter.*`）

```cpp
struct OsmImportError { std::string message; };

class OsmLaneletImporter {
public:
    static std::optional<LaneMapData> importFromFile(const std::string& path,
                                                     OsmImportError* error = nullptr);
    static std::optional<LaneMapData> importFromXmlString(const std::string& xml,
                                                          OsmImportError* error = nullptr);
};
```

流程：

1. 解析 node → 坐标表（id → x,y）
2. 解析 way → 折线点序列
3. 解析 lanelet relation → left/right way → **中点插值 centerline**
4. centerline 折线采样为 `LaneNode`；相邻采样点建 `LaneEdge`
5. **后继拓扑**：lanelet A 的 exit 端点 OSM node id 与 lanelet B 的 entry 端点 **相同** → A→B 有向边（禁止欧氏距离模糊匹配）

导入结果写入内存 `LaneMapData`，经既有 `LaneGraph::loadFromMap` 进入路由栈。可选另存为 JSON `lanes`。

### 4. UI 分层（详见 ADR-025）

- **OsmImportPanel**（独立 dock）：路径、预览摘要、导入按钮
- **MapImportPage**（Workbench Tab 7）：`map_source` / `osm_path` / 导入选项
- **禁止**在 MapEditorPanel / LaneEditorPanel 塞 OSM 控件

### 5. 测试与变异

| 测例 | 最低断言 |
|------|----------|
| `OsmLaneletImporterTest` | 合法 OSM → nodes≥2、edges≥1；坏 XML 失败；导入后 `shortestPath` 有值 |
| M43 | 丢弃 relation 或 centerline 恒空 → FAIL |

## 后果

- 正面：与 Autoware/Lanelet2 教学对齐；同一 `LaneGraph` API
- 负面：仅支持仓库内教学 `.osm` 方言；复杂路口需手工资产

## 禁止

- 禁止引入 Lanelet2 库
- 禁止 Domain `#include <Qt*>`
- 禁止解析器恒返回空图却标完成
- 禁止用几何距离推断 successor

## 参考

- ADR-002、ADR-018
- Lanelet2 OSM mapping README
- `docs/PHASE10_GOAL_PROMPT.md` §4.1
