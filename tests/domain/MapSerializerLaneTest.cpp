#include "domain/map/LaneGraph.h"
#include "domain/map/MapSerializer.h"
#include "domain/planning/LaneRouter.h"

#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

using fleetsim::domain::map::LaneEdge;
using fleetsim::domain::map::LaneGraph;
using fleetsim::domain::map::LaneMapData;
using fleetsim::domain::map::LaneNode;
using fleetsim::domain::map::MapDocument;
using fleetsim::domain::map::MapSerializer;
using fleetsim::domain::planning::LaneRouter;

namespace {

LaneMapData sampleLanes()
{
    LaneMapData lanes;
    lanes.nodes = {
        {"n0", 0.0, 0.0},
        {"n1", 4.0, 0.0},
        {"n2", 8.0, 0.0},
    };
    lanes.edges = {
        {"n0", "n1", false},
        {"n1", "n2", true},
    };
    return lanes;
}

}  // namespace

TEST(MapSerializerLaneTest, RoundTripLanesObject)
{
    MapDocument document;
    document.width_m = 12.0;
    document.height_m = 10.0;
    document.grid_resolution_m = 0.2;
    document.lanes = sampleLanes();

    const nlohmann::json json = MapSerializer::toJson(document);
    const MapDocument loaded = MapSerializer::fromJson(json);

    ASSERT_EQ(loaded.lanes.nodes.size(), 3U);
    ASSERT_EQ(loaded.lanes.edges.size(), 2U);
    EXPECT_EQ(loaded.lanes.nodes[1].id, "n1");
    EXPECT_DOUBLE_EQ(loaded.lanes.nodes[1].x, 4.0);
    EXPECT_EQ(loaded.lanes.edges[1].from, "n1");
    EXPECT_EQ(loaded.lanes.edges[1].to, "n2");
    EXPECT_TRUE(loaded.lanes.edges[1].bidirectional);
}

TEST(MapSerializerLaneTest, LegacyEmptyArrayLanes)
{
    nlohmann::json json = {
        {"version", 1},
        {"width_m", 20.0},
        {"height_m", 15.0},
        {"grid_resolution_m", 0.1},
        {"obstacles", nlohmann::json::array()},
        {"lanes", nlohmann::json::array()},
    };

    const MapDocument loaded = MapSerializer::fromJson(json);
    EXPECT_TRUE(loaded.lanes.nodes.empty());
    EXPECT_TRUE(loaded.lanes.edges.empty());
}

TEST(MapSerializerLaneTest, LoadedLanesBuildGraphAndRoute)
{
    MapDocument document;
    document.width_m = 10.0;
    document.height_m = 10.0;
    document.grid_resolution_m = 0.1;
    document.lanes = sampleLanes();

    const MapDocument loaded = MapSerializer::fromJson(MapSerializer::toJson(document));

    LaneGraph graph;
    ASSERT_TRUE(graph.loadFromMap(loaded.lanes));
    EXPECT_EQ(graph.nodeCount(), 3U);

    LaneRouter router(graph);
    const auto path = router.route("n0", "n2");
    ASSERT_TRUE(path.has_value());
    EXPECT_GE(path->size(), 2U);
    EXPECT_GT(path->length(), 0.0);
}

TEST(MapSerializerLaneTest, BidirectionalEdgeAllowsReverseRoute)
{
    MapDocument document;
    document.width_m = 10.0;
    document.height_m = 10.0;
    document.grid_resolution_m = 0.1;
    document.lanes.nodes = {{"a", 0.0, 0.0}, {"b", 2.0, 0.0}};
    document.lanes.edges = {{"a", "b", true}};

    const MapDocument loaded = MapSerializer::fromJson(MapSerializer::toJson(document));

    LaneGraph graph;
    ASSERT_TRUE(graph.loadFromMap(loaded.lanes));

    LaneRouter router(graph);
    EXPECT_TRUE(router.route("a", "b").has_value());
    EXPECT_TRUE(router.route("b", "a").has_value());
}
